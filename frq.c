//stm32实现高精度频率测定，捕获法（可精确到0.001Hz），定时器中断法（可精确到0.1Hz，两秒采集一次）
//一.捕获法
//现给出主要代码CaiJi.c
#include 'stm32f10x.h'
#include 'CaiJi.h'
//配置系统时钟,使能各外设时钟
void RCC_Configuration(void)
{
SystemInit();
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟配置
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO , ENABLE );
}
void GPIO_Configuration(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOF, &GPIO_InitStructure);
GPIO_SetBits(GPIOF,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void NVIC_Configuration(void)
{
NVIC_InitTypeDef NVIC_InitStructure;
#ifdef VECT_TAB_RAM
NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else
NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //NVIC配置
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);
}

void TIM3_Configuration(void)//TIM2初始化函数
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;

	TIM_DeInit(TIM3);
	TIM_TimeBaseStructure.TIM_Period = 0xffff;
	TIM_TimeBaseStructure.TIM_Prescaler = ?; //此值保密，呵呵，大家自己可以算一下，唯一奥，不然不准
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2; //通道选择
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising; //上升沿触发
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//管脚与寄存器对应关系
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	//输入预分频。意思是控制在多少个输入周期做一次捕获，如果
	//输入的信号频率没有变，测得的周期也不会变。比如选择4分频，则每四个输入周
	//期才做一次捕获，这样在输入信号变化不频繁的情况下，可以减少软件被不断中断的次数。
	TIM_ICInitStructure.TIM_ICFilter = 0x0; //滤波设置，经历几个周期跳变认定波形稳定0x0～0xF
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	//TIM_PWMIConfig(TIM3, &TIM_ICInitStructure); //根据参数配置TIM外设信息
	TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2); //选择IC2为始终触发源
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);//TIM从模式：触发信号的上升沿重新初始化计数器和触发寄存器的更新事件
	TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable); //启动定时器的被动触发
	TIM_Cmd(TIM3, ENABLE); //启动TIM2
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE); //打开中断使能CC1中断请求
}

stm32f10x_it.c文件
#include 'stm32f10x.h'
#include 'stm32f10x_it.h'
#include 'xpt2046.h'
#include 'delay.h'
extern int pinlv;
extern int caiji;
extern int m_timer;
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) == SET)
	{
		pinlv = TIM_GetCapture2(TIM3);
		caiji = 720000000/pinlv;
		//caiji=30000;
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2); //清楚TIM的中断待处理位
	}
}
//***************************************************************************************************************************************
//二.中断法 CaiJi.C文件
#include 'stm32f10x.h'
#include 'CaiJi.h'
//配置系统时钟,使能各外设时钟
void RCC_Configuration(void)
{
SystemInit();
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO , ENABLE );
}
void GPIO_Configuration(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOF, &GPIO_InitStructure);
GPIO_SetBits(GPIOF,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void Exit_Configuration()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
	//EXTI_InitStructure.EXTI_Line = EXTI_IMR_MR7;
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_GenerateSWInterrupt(EXTI_Line7);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void NVIC_Configuration(void)
{
#ifdef VECT_TAB_RAM
NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else
NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}
void Timerx_Init2()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	//TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_Period =9999;
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;
	TIM_TimeBaseStructure.TIM_RepetitionCounter =10 ;//只有高级定时器才有用
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	// TIM_ClearFlag(TIM2, TIM_FLAG_Update);清除更新标志位
	// TIM_ARRPreloadConfig(TIM2, DISABLE);//预装载寄存器的内容被立即传送到影子寄存器
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructu9999;
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;
	TIM_TimeBaseStructure.TIM_RepetitionCounter =10 ;//只有高级定时器才有用
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	// TIM_ClearFlag(TIM2, TIM_FLAG_Update);清除更新标志位
	// TIM_ARRPreloadConfig(TIM2, DISABLE);//预装载寄存器的内容被立即传送到影子寄存器
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//stm32f10x,it.h文件
#include 'stm32f10x.h'
#include 'xpt2046.h'
#include 'delay.h'
extern int pinlv;
extern int caiji;
extern int m_timer;

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);//测试用
		pinlv=pinlv+1;
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
}
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		m_timer=m_timer+1;
		if(m_timer==5)
		{
			//GPIO_SetBits(GPIOF,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);//测试用
			caiji=pinlv;
			pinlv=0;
			m_timer=0;
			//delay_m_timers(1000);
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
//好了 ，有疑问 我们在探讨！QQ：420055855