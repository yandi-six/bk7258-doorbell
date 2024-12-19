// 版权声明和许可证信息
/*
邮箱通道的驱动程序，支持物理和逻辑通道的初始化、打开、关闭、读写和控制操作。
通过使用中断服务程序和临界区保护，确保了多核环境下的安全通信。
*/
#include <stdio.h>
#include <string.h>

// 包含操作系统和邮箱通道的头文件
#include <os/os.h>
#include <driver/mailbox_channel.h>
#include "mailbox_driver_base.h"

// 定义物理命令和确认通道
#define MB_PHY_CMD_CHNL (MAILBOX_BOX0)
#define MB_PHY_ACK_CHNL (MAILBOX_BOX1)

// 定义通道状态
#define CHNL_STATE_BUSY 1
#define CHNL_STATE_ILDE 0

// 定义物理通道控制块结构体
typedef struct
{
	u8 tx_state;	  // 物理通道传输状态
	u8 tx_seq;		  // 物理通道传输序列
	u8 tx_log_chnl;	  // 逻辑通道
	u8 tx_hdr_cmd;	  // 传输头命令
	u32 rx_fault_cnt; // 接收故障计数
	u32 tx_fault_cnt; // 传输故障计数
} mb_phy_chnl_cb_t;

// 定义通道控制掩码
#define CHNL_CTRL_MASK 0xF

// 定义通道控制标志
#define CHNL_CTRL_ACK_BOX 0x01
#define CHNL_CTRL_SYNC_TX 0x02
#define CHNL_CTRL_RESET 0x04

// 定义物理通道头部的联合体
typedef union
{
	struct
	{
		u32 cmd : 8;		  // 命令
		u32 state : 4;		  // 状态
		u32 ctrl : 4;		  // 控制
		u32 tx_seq : 8;		  // 传输序列
		u32 logical_chnl : 8; // 逻辑通道
	};
	u32 data; // 数据
} phy_chnnl_hdr_t;

// 定义物理通道命令结构体
typedef struct
{
	phy_chnnl_hdr_t hdr; // 头部

	u32 param1; // 参数1
	u32 param2; // 参数2
	u32 param3; // 参数3
} mb_phy_chnl_cmd_t;

// 定义物理通道确认结构体
typedef struct
{
	phy_chnnl_hdr_t hdr; // 头部

	u32 data1; // 数据1
	u32 data2; // 数据2
	u32 data3; // 数据3
} mb_phy_chnl_ack_t;

// 定义物理通道联合体
typedef union
{
	phy_chnnl_hdr_t phy_ch_hdr;	  // 物理通道头部
	mb_phy_chnl_cmd_t phy_ch_cmd; // 物理通道命令
	mb_phy_chnl_ack_t phy_ch_ack; // 物理通道确认
	mailbox_data_t mb_data;		  // 邮箱数据
} mb_phy_chnl_union_t;

// 定义逻辑通道控制块结构体
typedef struct
{
	u8 tx_state;					// 逻辑通道传输状态
	u8 in_used;						// 是否在使用
	chnl_rx_isr_t rx_isr;			// 接收中断服务程序
	chnl_tx_isr_t tx_isr;			// 传输中断服务程序
	chnl_tx_cmpl_isr_t tx_cmpl_isr; // 传输完成中断服务程序
	void *isr_param;				// 中断服务程序参数
	mailbox_data_t chnnl_tx_buff;	// 逻辑通道传输缓冲区
} mb_log_chnl_cb_t;

// 定义物理通道数量
#define PHY_CHNL_NUM SYSTEM_CPU_NUM

// 定义物理通道控制块数组
static mb_phy_chnl_cb_t phy_chnl_x_cb[PHY_CHNL_NUM];

// 定义逻辑通道控制块数组
static mb_log_chnl_cb_t log_chnl_cb0[CP0_MB_LOG_CHNL_END - CP0_MB_LOG_CHNL_START];
static mb_log_chnl_cb_t log_chnl_cb1[CP1_MB_LOG_CHNL_END - CP1_MB_LOG_CHNL_START];
#if PHY_CHNL_NUM > 2
static mb_log_chnl_cb_t log_chnl_cb2[CP2_MB_LOG_CHNL_END - CP2_MB_LOG_CHNL_START];
#endif
#if PHY_CHNL_NUM > 3
static mb_log_chnl_cb_t log_chnl_cb3[CP3_MB_LOG_CHNL_END - CP3_MB_LOG_CHNL_START];
#endif

// 定义物理通道对应的逻辑通道数量
static const u8 phy_chnl_log_chnl_num[PHY_CHNL_NUM] = {
	ARRAY_SIZE(log_chnl_cb0),
	ARRAY_SIZE(log_chnl_cb1),
#if PHY_CHNL_NUM > 2
	ARRAY_SIZE(log_chnl_cb2),
#endif
#if PHY_CHNL_NUM > 3
	ARRAY_SIZE(log_chnl_cb3),
#endif
};

// 定义物理通道对应的逻辑通道列表
static const mb_log_chnl_cb_t *phy_chnl_log_chnl_list[PHY_CHNL_NUM] = {
	&log_chnl_cb0[0],
	&log_chnl_cb1[0],
#if PHY_CHNL_NUM > 2
	&log_chnl_cb2[0],
#endif
#if PHY_CHNL_NUM > 3
	&log_chnl_cb3[0],
#endif
};

// 定义通道初始化标志
static u8 mb_chnnl_init_ok = 0;

#ifdef CONFIG_FREERTOS_SMP
#include "spinlock.h"
static volatile spinlock_t mb_chnl_spin_lock = SPIN_LOCK_INIT;
#endif // CONFIG_FREERTOS_SMP

// 进入临界区
static inline uint32_t mb_chnl_enter_critical()
{
	uint32_t flags = rtos_disable_int();

#ifdef CONFIG_FREERTOS_SMP
	spin_lock(&mb_chnl_spin_lock);
#endif // CONFIG_FREERTOS_SMP

	return flags;
}

// 退出临界区
static inline void mb_chnl_exit_critical(uint32_t flags)
{
#ifdef CONFIG_FREERTOS_SMP
	spin_unlock(&mb_chnl_spin_lock);
#endif // CONFIG_FREERTOS_SMP

	rtos_enable_int(flags);
}

// 物理通道传输命令
static u8 mb_phy_chnl_tx_cmd(u8 log_chnl)
{
	mb_phy_chnl_cmd_t *cmd_ptr;
	bk_err_t ret_code;
	u16 chnl_type;
	u8 phy_chnl_idx = GET_DST_CPU_ID(log_chnl);	 // 获取目标CPU ID
	u8 log_chnl_idx = GET_LOG_CHNL_ID(log_chnl); // 获取逻辑通道ID

	mb_phy_chnl_cb_t *phy_chnl_ptr;
	mb_log_chnl_cb_t *log_chnl_cb_x;

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return 4;

	if (SELF_CPU == phy_chnl_idx) // 传输到自身
		return 5;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return 3;

	phy_chnl_ptr = &phy_chnl_x_cb[phy_chnl_idx];
	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
		return 1;

	phy_chnl_ptr->tx_seq++;
	phy_chnl_ptr->tx_log_chnl = log_chnl;

	cmd_ptr = (mb_phy_chnl_cmd_t *)&log_chnl_cb_x[log_chnl_idx].chnnl_tx_buff;

	cmd_ptr->hdr.logical_chnl = log_chnl;
	cmd_ptr->hdr.tx_seq = phy_chnl_ptr->tx_seq;
	cmd_ptr->hdr.ctrl = 0;
	cmd_ptr->hdr.state = 0;

	phy_chnl_ptr->tx_hdr_cmd = cmd_ptr->hdr.cmd;

	chnl_type = MB_PHY_CMD_CHNL;

	mailbox_endpoint_t dst_cpu = (mailbox_endpoint_t)(phy_chnl_idx);

	ret_code = bk_mailbox_send(&log_chnl_cb_x[log_chnl_idx].chnnl_tx_buff, SELF_CPU, dst_cpu, (void *)&chnl_type);

	if (ret_code != BK_OK)
	{
		phy_chnl_ptr->tx_fault_cnt++;

		return 2;
	}

	log_chnl_cb_x[log_chnl_idx].tx_state = CHNL_STATE_ILDE;

	if (log_chnl_cb_x[log_chnl_idx].tx_isr != NULL)
	{
		log_chnl_cb_x[log_chnl_idx].tx_isr(log_chnl_cb_x[log_chnl_idx].isr_param); // 物理通道现在忙，传输中断服务程序不会触发物理通道开始传输
	}

	return 0;
}

// 物理通道接收确认中断服务程序
static void mb_phy_chnl_rx_ack_isr(mb_phy_chnl_ack_t *ack_ptr)
{
	u8 log_chnl;
	u8 ret_code;
	u8 phy_chnl_idx;
	u8 log_chnl_idx;

	mb_phy_chnl_cb_t *phy_chnl_ptr;
	mb_log_chnl_cb_t *log_chnl_cb_x;

	log_chnl = ack_ptr->hdr.logical_chnl;

	phy_chnl_idx = GET_DST_CPU_ID(log_chnl); // 获取目标CPU ID
	log_chnl_idx = GET_LOG_CHNL_ID(log_chnl);

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return;

	if (SELF_CPU == phy_chnl_idx) // 从自身接收
		return;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return;

	phy_chnl_ptr = &phy_chnl_x_cb[phy_chnl_idx];
	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
		return;

#if CONFIG_SYS_CPU0
	if (ack_ptr->hdr.ctrl & CHNL_CTRL_RESET)
	{
		if (phy_chnl_ptr->tx_state == CHNL_STATE_ILDE)
		{
			goto try_to_start_tx;
		}

		ack_ptr->hdr.data = 0;
		ack_ptr->hdr.cmd = phy_chnl_ptr->tx_hdr_cmd;
		ack_ptr->hdr.state = CHNL_STATE_COM_FAIL;

		log_chnl_idx = GET_LOG_CHNL_ID(phy_chnl_ptr->tx_log_chnl);

		if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
			return;

		extern bk_err_t bk_mailbox_ready(mailbox_endpoint_t src, mailbox_endpoint_t dst, uint32_t box_id);
		if (bk_mailbox_ready(SELF_CPU, phy_chnl_idx, MB_PHY_CMD_CHNL) != BK_OK)
		{
			return;
		}

		log_chnl = phy_chnl_ptr->tx_log_chnl;
		ack_ptr->hdr.tx_seq = phy_chnl_ptr->tx_seq;

		ack_ptr->hdr.logical_chnl = log_chnl;

		ack_ptr->data1 = log_chnl_cb_x[log_chnl_idx].chnnl_tx_buff.param1;
		ack_ptr->data2 = log_chnl_cb_x[log_chnl_idx].chnnl_tx_buff.param2;
		ack_ptr->data3 = log_chnl_cb_x[log_chnl_idx].chnnl_tx_buff.param3;
	}
	else
#endif

		if ((log_chnl != phy_chnl_ptr->tx_log_chnl) ||
			(ack_ptr->hdr.tx_seq != phy_chnl_ptr->tx_seq))
	{
		phy_chnl_ptr->rx_fault_cnt++;

		return;
	}

	if (log_chnl_cb_x[log_chnl_idx].tx_cmpl_isr != NULL)
	{
		// 清除以下头部成员
		ack_ptr->hdr.logical_chnl = 0;
		ack_ptr->hdr.tx_seq = 0;
		ack_ptr->hdr.ctrl = 0;

		// hdr.state, hdr.cmd 这两个成员保持不变

		log_chnl_cb_x[log_chnl_idx].tx_cmpl_isr(log_chnl_cb_x[log_chnl_idx].isr_param, (mb_chnl_ack_t *)ack_ptr);
	}

#if CONFIG_SYS_CPU0
try_to_start_tx:
#endif

	for (log_chnl_idx = 0; log_chnl_idx < phy_chnl_log_chnl_num[phy_chnl_idx]; log_chnl_idx++) // 优先级降序搜索
	{
		if (log_chnl_cb_x[log_chnl_idx].tx_state != CHNL_STATE_ILDE)
			break;
	}

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
	{
		phy_chnl_ptr->tx_state = CHNL_STATE_ILDE;
		return;
	}

	log_chnl = CPX_LOG_CHNL_START(SELF_CPU, phy_chnl_idx) + log_chnl_idx;

	ret_code = mb_phy_chnl_tx_cmd(log_chnl);

	if (ret_code != 0)
	{
		phy_chnl_ptr->tx_state = CHNL_STATE_ILDE;
		return;
	}

	return;
}

// 物理通道接收命令中断服务程序
static void mb_phy_chnl_rx_cmd_isr(mb_phy_chnl_cmd_t *cmd_ptr)
{
	phy_chnnl_hdr_t chnl_hdr;
	u8 log_chnl = cmd_ptr->hdr.logical_chnl;
	bk_err_t ret_code;
	u16 chnl_type;

	u8 phy_chnl_idx;
	u8 log_chnl_idx;

	mb_phy_chnl_cb_t *phy_chnl_ptr;
	mb_log_chnl_cb_t *log_chnl_cb_x;

	phy_chnl_idx = GET_SRC_CPU_ID(log_chnl); // 获取源CPU ID
	log_chnl_idx = GET_LOG_CHNL_ID(log_chnl);

	if (SELF_CPU != GET_DST_CPU_ID(log_chnl))
		return;

	if (SELF_CPU == phy_chnl_idx) // 从自身接收
		return;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return;

	phy_chnl_ptr = &phy_chnl_x_cb[phy_chnl_idx];
	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	chnl_hdr.data = cmd_ptr->hdr.data;

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
	{
		phy_chnl_ptr->rx_fault_cnt++;

		return;
	}

	if (log_chnl_cb_x[log_chnl_idx].rx_isr != NULL)
	{
		// 清除所有其他头部成员，除了 hdr.cmd
		cmd_ptr->hdr.logical_chnl = 0;
		cmd_ptr->hdr.tx_seq = 0;
		cmd_ptr->hdr.ctrl = 0;
		cmd_ptr->hdr.state = 0;

		log_chnl_cb_x[log_chnl_idx].rx_isr(log_chnl_cb_x[log_chnl_idx].isr_param, (mb_chnl_cmd_t *)cmd_ptr);

		// cmd_ptr 缓冲区现在包含确认数据
	}
	else
	{
		chnl_hdr.state |= CHNL_STATE_COM_FAIL; // 命令没有目标应用程序，它是对等CPU的确认位
	}

	if (chnl_hdr.ctrl & CHNL_CTRL_SYNC_TX)
	{
		// 同步传输命令，不发送确认
		return;
	}

	// 物理通道传输确认

	// 重用命令缓冲区进行确认
	cmd_ptr->hdr.data = chnl_hdr.data;
	cmd_ptr->hdr.ctrl |= CHNL_CTRL_ACK_BOX; // 确认消息，使用确认通道

	chnl_type = MB_PHY_ACK_CHNL;

	mailbox_endpoint_t dst_cpu = (mailbox_endpoint_t)(phy_chnl_idx);

	ret_code = bk_mailbox_send((mailbox_data_t *)cmd_ptr, SELF_CPU, dst_cpu, (void *)&chnl_type); // 物理通道传输确认

	if (ret_code != BK_OK)
	{
		phy_chnl_ptr->tx_fault_cnt++;

		return;
	}

	return;
}

// 物理通道接收中断服务程序
static void mb_phy_chnl_rx_isr(mailbox_data_t *mb_data)
{
	mb_phy_chnl_union_t rx_data;

	rx_data.mb_data.param0 = mb_data->param0;
	rx_data.mb_data.param1 = mb_data->param1;
	rx_data.mb_data.param2 = mb_data->param2;
	rx_data.mb_data.param3 = mb_data->param3;
	// 以下过程将损坏输入参数，因此传入复制参数的指针而不是原始参数

	// 在一个MAILBOXn硬件中有两个盒子，但在当前邮箱驱动设计中无法知道这条消息来自哪个盒子
	// 因此使用消息头部中的 CHNL_CTRL_ACK_BOX 位来区分它来自哪里
	// 当 CHNL_CTRL_ACK_BOX 被设置时，表示来自确认盒子 (MAILBOXn_BOX1)
	if (rx_data.phy_ch_hdr.ctrl & CHNL_CTRL_ACK_BOX) // 接收确认
	{
		mb_phy_chnl_rx_ack_isr(&rx_data.phy_ch_ack);
	}
	else // 接收命令
	{
		mb_phy_chnl_rx_cmd_isr(&rx_data.phy_ch_cmd);
	}
}

// 物理通道开始传输
static void mb_phy_chnl_start_tx(u8 log_chnl)
{
	u8 ret_code;

	u8 phy_chnl_idx;

	mb_phy_chnl_cb_t *phy_chnl_ptr;

	phy_chnl_idx = GET_DST_CPU_ID(log_chnl); // 获取目标CPU ID

	if (SELF_CPU == phy_chnl_idx) // 传输到自身
		return;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return;

	phy_chnl_ptr = &phy_chnl_x_cb[phy_chnl_idx];

	if (phy_chnl_ptr->tx_state == CHNL_STATE_ILDE)
	{
		phy_chnl_ptr->tx_state = CHNL_STATE_BUSY; // 必须首先将通道状态设置为忙
		// start_tx->tx_cmd->tx_isr 回调->mb_chnl_write->start_tx，这是一个循环
		// 通过将 phy_chnl_cb.tx_state 设置为忙来打破循环

		ret_code = mb_phy_chnl_tx_cmd(log_chnl);

		if (ret_code != 0)
		{
			phy_chnl_ptr->tx_state = CHNL_STATE_ILDE;
		}
	}

	return;
}

// 物理通道同步传输命令
static bk_err_t mb_phy_chnl_tx_cmd_sync(u8 log_chnl, mb_phy_chnl_cmd_t *cmd_ptr)
{
	bk_err_t ret_code;
	u16 chnl_type;

	cmd_ptr->hdr.logical_chnl = log_chnl;
	cmd_ptr->hdr.tx_seq = 0;
	cmd_ptr->hdr.ctrl = CHNL_CTRL_SYNC_TX;
	cmd_ptr->hdr.state = 0;

	chnl_type = MB_PHY_CMD_CHNL;

	u8 phy_chnl_idx;

	phy_chnl_idx = GET_DST_CPU_ID(log_chnl); // 获取目标CPU ID

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return BK_ERR_PARAM;

	if (SELF_CPU == phy_chnl_idx) // 传输到自身
		return BK_ERR_PARAM;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return BK_ERR_PARAM;

	mailbox_endpoint_t dst_cpu = (mailbox_endpoint_t)(phy_chnl_idx);

	// 不能在这里等待 'phy_chnl_cb.tx_state' 变为 CHNL_STATE_ILDE
	// 'phy_chnl_cb.tx_state' 在中断回调中设置为 CHNL_STATE_ILDE
	// 但调用此API时可能会禁用中断
	// 通过轮询等待物理通道硬件空闲
	while (1)
	{
		ret_code = bk_mailbox_send((mailbox_data_t *)cmd_ptr, SELF_CPU, dst_cpu, (void *)&chnl_type);

		if (ret_code != BK_ERR_MAILBOX_TIMEOUT)
		{
			break;
		}
	}

	return ret_code;
}

// 物理通道传输重置
static bk_err_t mb_phy_chnl_tx_reset(u8 log_chnl)
{
	u8 phy_chnl_idx = GET_DST_CPU_ID(log_chnl); // 获取目标CPU ID

	mb_phy_chnl_cb_t *phy_chnl_ptr;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return BK_ERR_PARAM;

	phy_chnl_ptr = &phy_chnl_x_cb[phy_chnl_idx];

	u32 int_mask = mb_chnl_enter_critical();

	if ((phy_chnl_ptr->tx_state != CHNL_STATE_ILDE) &&
		(phy_chnl_ptr->tx_log_chnl == log_chnl))
	{
		phy_chnl_ptr->tx_state = CHNL_STATE_ILDE;
	}

	mb_chnl_exit_critical(int_mask);

	return BK_OK;
}

#if (CONFIG_SYS_CPU1)
// 物理通道重置
static bk_err_t mb_phy_chnl_reset(u8 dst_cpu)
{
	u16 chnl_type;
	mb_phy_chnl_cmd_t cmd_buf = {0};

	if (SELF_CPU == dst_cpu)
	{
		return BK_OK;
	}

	chnl_type = MB_PHY_ACK_CHNL;

	cmd_buf.hdr.data = 0;
	cmd_buf.hdr.ctrl |= (CHNL_CTRL_RESET | CHNL_CTRL_ACK_BOX);
	cmd_buf.hdr.logical_chnl = CPX_LOG_CHNL_START(dst_cpu, SELF_CPU);

	bk_mailbox_send((mailbox_data_t *)&cmd_buf, SELF_CPU, dst_cpu, (void *)&chnl_type);

	return BK_OK;
}
#endif

// 初始化逻辑通道模块
bk_err_t mb_chnl_init(void)
{
	bk_err_t ret_code;
	int i, j;

	mb_log_chnl_cb_t *log_chnl_cb_x;

	if (mb_chnnl_init_ok)
	{
		return BK_OK;
	}

	memset(&phy_chnl_x_cb, 0, sizeof(phy_chnl_x_cb));
	for (i = 0; i < PHY_CHNL_NUM; i++)
	{
		phy_chnl_x_cb[i].tx_state = CHNL_STATE_ILDE;

		log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[i]);

		memset(log_chnl_cb_x, 0, sizeof(mb_log_chnl_cb_t) * phy_chnl_log_chnl_num[i]);

		for (j = 0; j < phy_chnl_log_chnl_num[i]; j++)
		{
			log_chnl_cb_x[j].tx_state = CHNL_STATE_ILDE;
		}
	}

	ret_code = bk_mailbox_init();
	if (ret_code != BK_OK)
	{
		return ret_code;
	}

	mailbox_endpoint_t dst_cpu;

	for (i = 0; i < PHY_CHNL_NUM; i++)
	{
		dst_cpu = (mailbox_endpoint_t)(i); // 目标CPU ID

		bk_mailbox_recv_callback_register(dst_cpu, SELF_CPU, mb_phy_chnl_rx_isr);
	}

#if (CONFIG_SYS_CPU1)
	for (i = 0; i < PHY_CHNL_NUM; i++)
	{
		mb_phy_chnl_reset(i);
	}
#endif

	mb_chnnl_init_ok = 1;

	return BK_OK;
}

// 打开逻辑通道
bk_err_t mb_chnl_open(u8 log_chnl, void *callback_param)
{
	if (!mb_chnnl_init_ok) // 如果驱动未初始化
	{
		bk_err_t ret_code;

		ret_code = mb_chnl_init();

		if (ret_code != BK_OK)
		{
			return ret_code;
		}
	}

	u8 phy_chnl_idx = GET_DST_CPU_ID(log_chnl);	 // 获取目标CPU ID
	u8 log_chnl_idx = GET_LOG_CHNL_ID(log_chnl); // 获取逻辑通道ID

	mb_log_chnl_cb_t *log_chnl_cb_x;

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return BK_ERR_PARAM;

	if (SELF_CPU == phy_chnl_idx) // 传输到自身
		return BK_ERR_PARAM;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return BK_ERR_PARAM;

	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
		return BK_ERR_PARAM;

	if (log_chnl_cb_x[log_chnl_idx].in_used)
		return BK_ERR_OPEN;

	log_chnl_cb_x[log_chnl_idx].in_used = 1; // 通道正在使用
	log_chnl_cb_x[log_chnl_idx].isr_param = callback_param;

	return BK_OK;
}

// 关闭逻辑通道
bk_err_t mb_chnl_close(u8 log_chnl)
{
	u8 phy_chnl_idx = GET_DST_CPU_ID(log_chnl);	 // 获取目标CPU ID
	u8 log_chnl_idx = GET_LOG_CHNL_ID(log_chnl); // 获取逻辑通道ID

	mb_log_chnl_cb_t *log_chnl_cb_x;

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return BK_ERR_PARAM;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return BK_ERR_PARAM;

	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
		return BK_ERR_PARAM;

	if (log_chnl_cb_x[log_chnl_idx].in_used == 0)
		return BK_ERR_STATE;

	log_chnl_cb_x[log_chnl_idx].in_used = 0;
	log_chnl_cb_x[log_chnl_idx].tx_state = CHNL_STATE_ILDE;
	log_chnl_cb_x[log_chnl_idx].rx_isr = NULL;
	log_chnl_cb_x[log_chnl_idx].tx_isr = NULL;
	log_chnl_cb_x[log_chnl_idx].tx_cmpl_isr = NULL;

	mb_phy_chnl_tx_reset(log_chnl);

	return BK_OK;
}

// 从逻辑通道读取
bk_err_t mb_chnl_read(u8 log_chnl, mb_chnl_cmd_t *read_buf)
{
	return BK_ERR_NOT_SUPPORT;
}

// 写入逻辑通道
bk_err_t mb_chnl_write(u8 log_chnl, mb_chnl_cmd_t *cmd_buf)
{
	u16 write_len;

	u8 phy_chnl_idx = GET_DST_CPU_ID(log_chnl);	 // 获取目标CPU ID
	u8 log_chnl_idx = GET_LOG_CHNL_ID(log_chnl); // 获取逻辑通道ID

	mb_log_chnl_cb_t *log_chnl_cb_x;

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return BK_ERR_PARAM;

	if (SELF_CPU == phy_chnl_idx) // 传输到自身
		return BK_ERR_PARAM;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return BK_ERR_PARAM;

	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
		return BK_ERR_PARAM;

	if (log_chnl_cb_x[log_chnl_idx].in_used == 0)
		return BK_ERR_STATE;

	u32 int_mask = mb_chnl_enter_critical();

	if (log_chnl_cb_x[log_chnl_idx].tx_state != CHNL_STATE_ILDE)
	{
		mb_chnl_exit_critical(int_mask);

		return BK_ERR_BUSY;
	}

	write_len = sizeof(mailbox_data_t) > sizeof(mb_chnl_cmd_t) ? sizeof(mb_chnl_cmd_t) : sizeof(mailbox_data_t);

	memcpy(&log_chnl_cb_x[log_chnl_idx].chnnl_tx_buff, cmd_buf, write_len);

	// 设置为忙表示传输缓冲区中有数据。mb_phy_chnl_rx_ack_isr 将获取它进行发送
	log_chnl_cb_x[log_chnl_idx].tx_state = CHNL_STATE_BUSY; // 必须在数据复制后设置为忙

	mb_phy_chnl_start_tx(log_chnl);

	mb_chnl_exit_critical(int_mask);

	return BK_OK;
}

// 逻辑通道杂项IO（设置/获取参数）
bk_err_t mb_chnl_ctrl(u8 log_chnl, u8 cmd, void *param)
{
	bk_err_t ret_code;

	u8 phy_chnl_idx = GET_DST_CPU_ID(log_chnl);	 // 获取目标CPU ID
	u8 log_chnl_idx = GET_LOG_CHNL_ID(log_chnl); // 获取逻辑通道ID

	mb_log_chnl_cb_t *log_chnl_cb_x;

	if (SELF_CPU != GET_SRC_CPU_ID(log_chnl))
		return BK_ERR_PARAM;

	if (SELF_CPU == phy_chnl_idx) // 传输到自身
		return BK_ERR_PARAM;

	if (phy_chnl_idx >= PHY_CHNL_NUM)
		return BK_ERR_PARAM;

	log_chnl_cb_x = (mb_log_chnl_cb_t *)(phy_chnl_log_chnl_list[phy_chnl_idx]);

	if (log_chnl_idx >= phy_chnl_log_chnl_num[phy_chnl_idx])
		return BK_ERR_PARAM;

	if (log_chnl_cb_x[log_chnl_idx].in_used == 0)
		return BK_ERR_STATE;

	switch (cmd)
	{
	case MB_CHNL_GET_STATUS:

		if (param == NULL)
			return BK_ERR_NULL_PARAM;

		*((u8 *)param) = log_chnl_cb_x[log_chnl_idx].tx_state;

		break;

	case MB_CHNL_SET_RX_ISR:
		log_chnl_cb_x[log_chnl_idx].rx_isr = (chnl_rx_isr_t)param;
		break;

	case MB_CHNL_SET_TX_ISR:
		log_chnl_cb_x[log_chnl_idx].tx_isr = (chnl_tx_isr_t)param;
		break;

	case MB_CHNL_SET_TX_CMPL_ISR:
		log_chnl_cb_x[log_chnl_idx].tx_cmpl_isr = (chnl_tx_cmpl_isr_t)param;
		break;

	case MB_CHNL_WRITE_SYNC:
		if (param == NULL)
			return BK_ERR_NULL_PARAM;

		ret_code = mb_phy_chnl_tx_cmd_sync(log_chnl, (mb_phy_chnl_cmd_t *)param);
		return ret_code;
		break;

	case MB_CHNL_TX_RESET:
		log_chnl_cb_x[log_chnl_idx].tx_state = CHNL_STATE_ILDE;
		ret_code = mb_phy_chnl_tx_reset(log_chnl);
		return ret_code;
		break;

	default:
		return BK_ERR_NOT_SUPPORT;
		break;
	}

	return BK_OK;
}