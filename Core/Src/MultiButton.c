#include "MultiButton.h"
#include "stdio.h"
#define EVENT_CB(ev)    \
	if (handle->cb[ev]) \
	handle->cb[ev]((Button *)handle)

static struct Button *head_handle = NULL;
/**
 * @brief  Initializes the button struct handle.
 * @param  handle: the button handle strcut.
 * @param  pin_level: read the HAL GPIO of the connet button level.
 * @param  active_level: pressed GPIO level.
 * @retval None
 */
void button_init(struct Button *handle, uint8_t (*pin_level)(), uint8_t act_level)
{
	memset(handle, 0, sizeof(struct Button)); //以这个handle为起点，将Button结构体大小的内存清空为0
	handle->event = (uint8_t)NONE_PRESS;
	handle->hal_button_level = pin_level;			   //此处传入的是一个函数地址，给结构体中的函数。
	handle->button_level = handle->hal_button_level(); //此处调用结构体中的函数 也就是传入的那个函数
	handle->active_level = act_level;
}
/**
 * @brief  Attach the button event callback function.
 * @param  handle: the button handle strcut.
 * @param  event: trigger event type.
 * @param  cb: callback function.
 * @retval None
 */
void button_attach(struct Button *handle, PressEvent event, BtnCallback cb)
{
	//将按键结构体中的回调函数与传入的事件与回调函数关联。
	handle->cb[event] = cb;
}
/**
 * @brief  Inquire the button event happen.
 * @param  handle: the button handle strcut.
 * @retval button event.
 */
PressEvent get_button_event(struct Button *handle)
{
	return (PressEvent)handle->event;
}
/**
 * @brief  Button driver core function, driver state machine.
 * @param  handle: the button handle strcut.
 * @retval None
 */
void button_handler(struct Button *handle)
{
	uint8_t read_gpio_level = handle->hal_button_level(); //获取gpio电平

	// ticks counter working...
	if ((handle->state) > 0)
		handle->ticks++;

	/*button debounce handle*/
	if (read_gpio_level != handle->button_level)
	{ // not equal to prev one
		// continue read 3 times same new level change
		if (++(handle->debounce_cnt) >= DEBOUNCE_TICKS)
		{
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	}
	else
	{ // leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state)
	{
	case 0:
		if (handle->button_level == handle->active_level)
		{ // start press down
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN); //#define EVENT_CB(ev)    if(handle->cb[ev])handle->cb[ev]((Button *)handle)
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		}
		else
		{
			handle->event = (uint8_t)NONE_PRESS;
		}
		break;

	case 1:
		if (handle->button_level != handle->active_level)
		{ // released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;
		}
		else if (handle->ticks > LONG_TICKS)
		{
			handle->event = (uint8_t)LONG_PRESS_START;
			EVENT_CB(LONG_PRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if (handle->button_level == handle->active_level)
		{ // press down again
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		}
		else if (handle->ticks > SHORT_TICKS)
		{ // released timeout
			if (handle->repeat == 1)
			{
				handle->event = (uint8_t)SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
			}
			else if (handle->repeat == 2)
			{
				handle->event = (uint8_t)DOUBLE_CLICK;
				EVENT_CB(DOUBLE_CLICK); // repeat hit
			}
			handle->state = 0;
		}
		break;

	case 3:
		if (handle->button_level != handle->active_level)
		{ // released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			if (handle->ticks < SHORT_TICKS)
			{
				handle->ticks = 0;
				handle->state = 2; // repeat press
			}
			else
			{
				handle->state = 0;
			}
		}
		else if (handle->ticks > SHORT_TICKS)
		{ // long press up
			handle->state = 0;
		}
		break;

	case 5:
		if (handle->button_level == handle->active_level)
		{
			// continue hold trigger
			handle->event = (uint8_t)LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);
		}
		else
		{ // releasd
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; // reset
		}
		break;
	}
}
/**
 * @brief  Start the button work, add the handle into work list.
 * @param  handle: target handle strcut.
 * @retval 0: succeed. -1: already exist.
 */
int button_start(struct Button *handle)
{
	struct Button *target = head_handle; //申请一个指针指向头结点
	while (target)						 // target是一个临时的链表中结点的容器。
	{
		if (target == handle)  //检查传入结点地址与链表中的结点地址有没有重复的
			return -1;		   //如果有重置的结点地址就代表这个结点已经存在于链表中，直接返回-1
		target = target->next; //链表向后遍历，与传入结点进行比对
	}

	handle->next = head_handle; //传入的结点的尾指针，指向头结点。
	head_handle = handle;		//头结点变为当前传入的结点。
}

/**
 * @brief  Stop the button work, remove the handle off work list.
 * @param  handle: target handle strcut.
 * @retval None
 */
void button_stop(struct Button *handle)
{
	struct Button **curr;
	for (curr = &head_handle; *curr;)
	{
		struct Button *entry = *curr;
		if (entry == handle)
		{
			*curr = entry->next;
			//			free(entry);
			return; // glacier add 2021-8-18
		}
		else
			curr = &entry->next;
	}
}

/**
 * @brief  background ticks, timer repeat invoking interval 5ms.
 * @param  None.
 * @retval None.
 */
void button_ticks()
{
	struct Button *target;
	for (target = head_handle; target; target = target->next)
	{
		button_handler(target);
	}
}