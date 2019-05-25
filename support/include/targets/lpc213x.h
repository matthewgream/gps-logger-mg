// LPC213x register and bit definitions.
//
// Copyright (c) 2005 Rowley Associates Limited.
//
// This file may be distributed under the terms of the License Agreement
// provided with this software.
//
// THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#ifndef LPC213x_h
#define LPC213x_h

#define WDMOD (*(volatile unsigned long *)0xE0000000)
#define WDMOD_WDEN_MASK 0x1U
#define WDMOD_WDEN 0x1U
#define WDMOD_WDEN_BIT 0
#define WDMOD_WDRESET_MASK 0x2U
#define WDMOD_WDRESET 0x2U
#define WDMOD_WDRESET_BIT 1
#define WDMOD_WDTOF_MASK 0x4U
#define WDMOD_WDTOF 0x4U
#define WDMOD_WDTOF_BIT 2
#define WDMOD_WDINT_MASK 0x8U
#define WDMOD_WDINT 0x8U
#define WDMOD_WDINT_BIT 3

#define WDTC (*(volatile unsigned long *)0xE0000004)

#define WDFEED (*(volatile unsigned long *)0xE0000008)

#define WDTV (*(volatile unsigned long *)0xE000000C)

#define T0IR (*(volatile unsigned long *)0xE0004000)
#define T0IR_MR0_MASK 0x1U
#define T0IR_MR0 0x1U
#define T0IR_MR0_BIT 0
#define T0IR_MR1_MASK 0x2U
#define T0IR_MR1 0x2U
#define T0IR_MR1_BIT 1
#define T0IR_MR2_MASK 0x4U
#define T0IR_MR2 0x4U
#define T0IR_MR2_BIT 2
#define T0IR_MR3_MASK 0x8U
#define T0IR_MR3 0x8U
#define T0IR_MR3_BIT 3
#define T0IR_CR0_MASK 0x10U
#define T0IR_CR0 0x10U
#define T0IR_CR0_BIT 4
#define T0IR_CR1_MASK 0x20U
#define T0IR_CR1 0x20U
#define T0IR_CR1_BIT 5
#define T0IR_CR2_MASK 0x40U
#define T0IR_CR2 0x40U
#define T0IR_CR2_BIT 6
#define T0IR_CR3_MASK 0x80U
#define T0IR_CR3 0x80U
#define T0IR_CR3_BIT 7

#define T0TCR (*(volatile unsigned long *)0xE0004004)
#define T0TCR_Counter_Enable_MASK 0x1U
#define T0TCR_Counter_Enable 0x1U
#define T0TCR_Counter_Enable_BIT 0
#define T0TCR_Counter_Reset_MASK 0x2U
#define T0TCR_Counter_Reset 0x2U
#define T0TCR_Counter_Reset_BIT 1

#define T0TC (*(volatile unsigned long *)0xE0004008)

#define T0PR (*(volatile unsigned long *)0xE000400C)

#define T0PC (*(volatile unsigned long *)0xE0004010)

#define T0MCR (*(volatile unsigned long *)0xE0004014)
#define T0MCR_Interrupt_on_MR0_MASK 0x1U
#define T0MCR_Interrupt_on_MR0 0x1U
#define T0MCR_Interrupt_on_MR0_BIT 0
#define T0MCR_Reset_on_MR0_MASK 0x2U
#define T0MCR_Reset_on_MR0 0x2U
#define T0MCR_Reset_on_MR0_BIT 1
#define T0MCR_Stop_on_MR0_MASK 0x4U
#define T0MCR_Stop_on_MR0 0x4U
#define T0MCR_Stop_on_MR0_BIT 2
#define T0MCR_Interrupt_on_MR1_MASK 0x8U
#define T0MCR_Interrupt_on_MR1 0x8U
#define T0MCR_Interrupt_on_MR1_BIT 3
#define T0MCR_Reset_on_MR1_MASK 0x10U
#define T0MCR_Reset_on_MR1 0x10U
#define T0MCR_Reset_on_MR1_BIT 4
#define T0MCR_Stop_on_MR1_MASK 0x20U
#define T0MCR_Stop_on_MR1 0x20U
#define T0MCR_Stop_on_MR1_BIT 5
#define T0MCR_Interrupt_on_MR2_MASK 0x80U
#define T0MCR_Interrupt_on_MR2 0x80U
#define T0MCR_Interrupt_on_MR2_BIT 7
#define T0MCR_Reset_on_MR2_MASK 0x80U
#define T0MCR_Reset_on_MR2 0x80U
#define T0MCR_Reset_on_MR2_BIT 7
#define T0MCR_Stop_on_MR2_MASK 0x100U
#define T0MCR_Stop_on_MR2 0x100U
#define T0MCR_Stop_on_MR2_BIT 8
#define T0MCR_Interrupt_on_MR3_MASK 0x200U
#define T0MCR_Interrupt_on_MR3 0x200U
#define T0MCR_Interrupt_on_MR3_BIT 9
#define T0MCR_Reset_on_MR3_MASK 0x400U
#define T0MCR_Reset_on_MR3 0x400U
#define T0MCR_Reset_on_MR3_BIT 10
#define T0MCR_Stop_on_MR3_MASK 0x800U
#define T0MCR_Stop_on_MR3 0x800U
#define T0MCR_Stop_on_MR3_BIT 11

#define T0MR0 (*(volatile unsigned long *)0xE0004018)

#define T0MR1 (*(volatile unsigned long *)0xE000401C)

#define T0MR2 (*(volatile unsigned long *)0xE0004020)

#define T0MR3 (*(volatile unsigned long *)0xE0004024)

#define T0CCR (*(volatile unsigned long *)0xE0004028)
#define T0CCR_Capture_on_CAPn_0_rising_edge_MASK 0x1U
#define T0CCR_Capture_on_CAPn_0_rising_edge 0x1U
#define T0CCR_Capture_on_CAPn_0_rising_edge_BIT 0
#define T0CCR_Capture_on_CAPn_0_falling_edge_MASK 0x2U
#define T0CCR_Capture_on_CAPn_0_falling_edge 0x2U
#define T0CCR_Capture_on_CAPn_0_falling_edge_BIT 1
#define T0CCR_Interrupt_on_CAPn_0_event_MASK 0x4U
#define T0CCR_Interrupt_on_CAPn_0_event 0x4U
#define T0CCR_Interrupt_on_CAPn_0_event_BIT 2
#define T0CCR_Capture_on_CAPn_1_rising_edge_MASK 0x8U
#define T0CCR_Capture_on_CAPn_1_rising_edge 0x8U
#define T0CCR_Capture_on_CAPn_1_rising_edge_BIT 3
#define T0CCR_Capture_on_CAPn_1_falling_edge_MASK 0x10U
#define T0CCR_Capture_on_CAPn_1_falling_edge 0x10U
#define T0CCR_Capture_on_CAPn_1_falling_edge_BIT 4
#define T0CCR_Interrupt_on_CAPn_1_event_MASK 0x20U
#define T0CCR_Interrupt_on_CAPn_1_event 0x20U
#define T0CCR_Interrupt_on_CAPn_1_event_BIT 5
#define T0CCR_Capture_on_CAPn_2_rising_edge_MASK 0x80U
#define T0CCR_Capture_on_CAPn_2_rising_edge 0x80U
#define T0CCR_Capture_on_CAPn_2_rising_edge_BIT 7
#define T0CCR_Capture_on_CAPn_2_falling_edge_MASK 0x80U
#define T0CCR_Capture_on_CAPn_2_falling_edge 0x80U
#define T0CCR_Capture_on_CAPn_2_falling_edge_BIT 7
#define T0CCR_Interrupt_on_CAPn_2_event_MASK 0x100U
#define T0CCR_Interrupt_on_CAPn_2_event 0x100U
#define T0CCR_Interrupt_on_CAPn_2_event_BIT 8
#define T0CCR_Capture_on_CAPn_3_rising_edge_MASK 0x200U
#define T0CCR_Capture_on_CAPn_3_rising_edge 0x200U
#define T0CCR_Capture_on_CAPn_3_rising_edge_BIT 9
#define T0CCR_Capture_on_CAPn_3_falling_edge_MASK 0x400U
#define T0CCR_Capture_on_CAPn_3_falling_edge 0x400U
#define T0CCR_Capture_on_CAPn_3_falling_edge_BIT 10
#define T0CCR_Interrupt_on_CAPn_3_event_MASK 0x800U
#define T0CCR_Interrupt_on_CAPn_3_event 0x800U
#define T0CCR_Interrupt_on_CAPn_3_event_BIT 11

#define T0CR0 (*(volatile unsigned long *)0xE000402C)

#define T0CR1 (*(volatile unsigned long *)0xE0004030)

#define T0CR2 (*(volatile unsigned long *)0xE0004034)

#define T0CR3 (*(volatile unsigned long *)0xE0004038)

#define T0EMR (*(volatile unsigned long *)0xE000403C)
#define T0EMR_External_Match_0_MASK 0x1U
#define T0EMR_External_Match_0 0x1U
#define T0EMR_External_Match_0_BIT 0
#define T0EMR_External_Match_1_MASK 0x2U
#define T0EMR_External_Match_1 0x2U
#define T0EMR_External_Match_1_BIT 1
#define T0EMR_External_Match_2_MASK 0x4U
#define T0EMR_External_Match_2 0x4U
#define T0EMR_External_Match_2_BIT 2
#define T0EMR_External_Match_3_MASK 0x8U
#define T0EMR_External_Match_3 0x8U
#define T0EMR_External_Match_3_BIT 3
#define T0EMR_External_Match_Control_0_MASK 0x30U
#define T0EMR_External_Match_Control_0_BIT 4
#define T0EMR_External_Match_Control_1_MASK 0xC0U
#define T0EMR_External_Match_Control_1_BIT 6
#define T0EMR_External_Match_Control_2_MASK 0x300U
#define T0EMR_External_Match_Control_2_BIT 8
#define T0EMR_External_Match_Control_3_MASK 0xC00U
#define T0EMR_External_Match_Control_3_BIT 10

#define T0CTCR (*(volatile unsigned long *)0xE0004070)
#define T0CTCR_Mode_MASK 0x3U
#define T0CTCR_Mode_BIT 0
#define T0CTCR_Input_Select_MASK 0xCU
#define T0CTCR_Input_Select_BIT 2

#define T1IR (*(volatile unsigned long *)0xE0008000)
#define T1IR_MR0_MASK 0x1U
#define T1IR_MR0 0x1U
#define T1IR_MR0_BIT 0
#define T1IR_MR1_MASK 0x2U
#define T1IR_MR1 0x2U
#define T1IR_MR1_BIT 1
#define T1IR_MR2_MASK 0x4U
#define T1IR_MR2 0x4U
#define T1IR_MR2_BIT 2
#define T1IR_MR3_MASK 0x8U
#define T1IR_MR3 0x8U
#define T1IR_MR3_BIT 3
#define T1IR_CR0_MASK 0x10U
#define T1IR_CR0 0x10U
#define T1IR_CR0_BIT 4
#define T1IR_CR1_MASK 0x20U
#define T1IR_CR1 0x20U
#define T1IR_CR1_BIT 5
#define T1IR_CR2_MASK 0x40U
#define T1IR_CR2 0x40U
#define T1IR_CR2_BIT 6
#define T1IR_CR3_MASK 0x80U
#define T1IR_CR3 0x80U
#define T1IR_CR3_BIT 7

#define T1TCR (*(volatile unsigned long *)0xE0008004)
#define T1TCR_Counter_Enable_MASK 0x1U
#define T1TCR_Counter_Enable 0x1U
#define T1TCR_Counter_Enable_BIT 0
#define T1TCR_Counter_Reset_MASK 0x2U
#define T1TCR_Counter_Reset 0x2U
#define T1TCR_Counter_Reset_BIT 1

#define T1TC (*(volatile unsigned long *)0xE0008008)

#define T1PR (*(volatile unsigned long *)0xE000800C)

#define T1PC (*(volatile unsigned long *)0xE0008010)

#define T1MCR (*(volatile unsigned long *)0xE0008014)
#define T1MCR_Interrupt_on_MR0_MASK 0x1U
#define T1MCR_Interrupt_on_MR0 0x1U
#define T1MCR_Interrupt_on_MR0_BIT 0
#define T1MCR_Reset_on_MR0_MASK 0x2U
#define T1MCR_Reset_on_MR0 0x2U
#define T1MCR_Reset_on_MR0_BIT 1
#define T1MCR_Stop_on_MR0_MASK 0x4U
#define T1MCR_Stop_on_MR0 0x4U
#define T1MCR_Stop_on_MR0_BIT 2
#define T1MCR_Interrupt_on_MR1_MASK 0x8U
#define T1MCR_Interrupt_on_MR1 0x8U
#define T1MCR_Interrupt_on_MR1_BIT 3
#define T1MCR_Reset_on_MR1_MASK 0x10U
#define T1MCR_Reset_on_MR1 0x10U
#define T1MCR_Reset_on_MR1_BIT 4
#define T1MCR_Stop_on_MR1_MASK 0x20U
#define T1MCR_Stop_on_MR1 0x20U
#define T1MCR_Stop_on_MR1_BIT 5
#define T1MCR_Interrupt_on_MR2_MASK 0x80U
#define T1MCR_Interrupt_on_MR2 0x80U
#define T1MCR_Interrupt_on_MR2_BIT 7
#define T1MCR_Reset_on_MR2_MASK 0x80U
#define T1MCR_Reset_on_MR2 0x80U
#define T1MCR_Reset_on_MR2_BIT 7
#define T1MCR_Stop_on_MR2_MASK 0x100U
#define T1MCR_Stop_on_MR2 0x100U
#define T1MCR_Stop_on_MR2_BIT 8
#define T1MCR_Interrupt_on_MR3_MASK 0x200U
#define T1MCR_Interrupt_on_MR3 0x200U
#define T1MCR_Interrupt_on_MR3_BIT 9
#define T1MCR_Reset_on_MR3_MASK 0x400U
#define T1MCR_Reset_on_MR3 0x400U
#define T1MCR_Reset_on_MR3_BIT 10
#define T1MCR_Stop_on_MR3_MASK 0x800U
#define T1MCR_Stop_on_MR3 0x800U
#define T1MCR_Stop_on_MR3_BIT 11

#define T1MR0 (*(volatile unsigned long *)0xE0008018)

#define T1MR1 (*(volatile unsigned long *)0xE000801C)

#define T1MR2 (*(volatile unsigned long *)0xE0008020)

#define T1MR3 (*(volatile unsigned long *)0xE0008024)

#define T1CCR (*(volatile unsigned long *)0xE0008028)
#define T1CCR_Capture_on_CAPn_0_rising_edge_MASK 0x1U
#define T1CCR_Capture_on_CAPn_0_rising_edge 0x1U
#define T1CCR_Capture_on_CAPn_0_rising_edge_BIT 0
#define T1CCR_Capture_on_CAPn_0_falling_edge_MASK 0x2U
#define T1CCR_Capture_on_CAPn_0_falling_edge 0x2U
#define T1CCR_Capture_on_CAPn_0_falling_edge_BIT 1
#define T1CCR_Interrupt_on_CAPn_0_event_MASK 0x4U
#define T1CCR_Interrupt_on_CAPn_0_event 0x4U
#define T1CCR_Interrupt_on_CAPn_0_event_BIT 2
#define T1CCR_Capture_on_CAPn_1_rising_edge_MASK 0x8U
#define T1CCR_Capture_on_CAPn_1_rising_edge 0x8U
#define T1CCR_Capture_on_CAPn_1_rising_edge_BIT 3
#define T1CCR_Capture_on_CAPn_1_falling_edge_MASK 0x10U
#define T1CCR_Capture_on_CAPn_1_falling_edge 0x10U
#define T1CCR_Capture_on_CAPn_1_falling_edge_BIT 4
#define T1CCR_Interrupt_on_CAPn_1_event_MASK 0x20U
#define T1CCR_Interrupt_on_CAPn_1_event 0x20U
#define T1CCR_Interrupt_on_CAPn_1_event_BIT 5
#define T1CCR_Capture_on_CAPn_2_rising_edge_MASK 0x80U
#define T1CCR_Capture_on_CAPn_2_rising_edge 0x80U
#define T1CCR_Capture_on_CAPn_2_rising_edge_BIT 7
#define T1CCR_Capture_on_CAPn_2_falling_edge_MASK 0x80U
#define T1CCR_Capture_on_CAPn_2_falling_edge 0x80U
#define T1CCR_Capture_on_CAPn_2_falling_edge_BIT 7
#define T1CCR_Interrupt_on_CAPn_2_event_MASK 0x100U
#define T1CCR_Interrupt_on_CAPn_2_event 0x100U
#define T1CCR_Interrupt_on_CAPn_2_event_BIT 8
#define T1CCR_Capture_on_CAPn_3_rising_edge_MASK 0x200U
#define T1CCR_Capture_on_CAPn_3_rising_edge 0x200U
#define T1CCR_Capture_on_CAPn_3_rising_edge_BIT 9
#define T1CCR_Capture_on_CAPn_3_falling_edge_MASK 0x400U
#define T1CCR_Capture_on_CAPn_3_falling_edge 0x400U
#define T1CCR_Capture_on_CAPn_3_falling_edge_BIT 10
#define T1CCR_Interrupt_on_CAPn_3_event_MASK 0x800U
#define T1CCR_Interrupt_on_CAPn_3_event 0x800U
#define T1CCR_Interrupt_on_CAPn_3_event_BIT 11

#define T1CR0 (*(volatile unsigned long *)0xE000802C)

#define T1CR1 (*(volatile unsigned long *)0xE0008030)

#define T1CR2 (*(volatile unsigned long *)0xE0008034)

#define T1CR3 (*(volatile unsigned long *)0xE0008038)

#define T1EMR (*(volatile unsigned long *)0xE000803C)
#define T1EMR_External_Match_0_MASK 0x1U
#define T1EMR_External_Match_0 0x1U
#define T1EMR_External_Match_0_BIT 0
#define T1EMR_External_Match_1_MASK 0x2U
#define T1EMR_External_Match_1 0x2U
#define T1EMR_External_Match_1_BIT 1
#define T1EMR_External_Match_2_MASK 0x4U
#define T1EMR_External_Match_2 0x4U
#define T1EMR_External_Match_2_BIT 2
#define T1EMR_External_Match_3_MASK 0x8U
#define T1EMR_External_Match_3 0x8U
#define T1EMR_External_Match_3_BIT 3
#define T1EMR_External_Match_Control_0_MASK 0x30U
#define T1EMR_External_Match_Control_0_BIT 4
#define T1EMR_External_Match_Control_1_MASK 0xC0U
#define T1EMR_External_Match_Control_1_BIT 6
#define T1EMR_External_Match_Control_2_MASK 0x300U
#define T1EMR_External_Match_Control_2_BIT 8
#define T1EMR_External_Match_Control_3_MASK 0xC00U
#define T1EMR_External_Match_Control_3_BIT 10

#define T1CTCR (*(volatile unsigned long *)0xE0008070)
#define T1CTCR_Mode_MASK 0x3U
#define T1CTCR_Mode_BIT 0
#define T1CTCR_Input_Select_MASK 0xCU
#define T1CTCR_Input_Select_BIT 2

#define U0RBR (*(volatile unsigned char *)0xE000C000)

#define U0THR (*(volatile unsigned char *)0xE000C000)

#define U0DLL (*(volatile unsigned char *)0xE000C000)

#define U0DLM (*(volatile unsigned char *)0xE000C004)

#define U0IER (*(volatile unsigned char *)0xE000C004)
#define U0IER_RBR_Interrupt_Enable_MASK 0x1U
#define U0IER_RBR_Interrupt_Enable 0x1U
#define U0IER_RBR_Interrupt_Enable_BIT 0
#define U0IER_THRE_Interrupt_Enable_MASK 0x2U
#define U0IER_THRE_Interrupt_Enable 0x2U
#define U0IER_THRE_Interrupt_Enable_BIT 1
#define U0IER_Rx_Line_Status_Interrupt_Enable_MASK 0x4U
#define U0IER_Rx_Line_Status_Interrupt_Enable 0x4U
#define U0IER_Rx_Line_Status_Interrupt_Enable_BIT 2

#define U0IIR (*(volatile unsigned char *)0xE000C008)
#define U0IIR_Interrupt_Pending_MASK 0x1U
#define U0IIR_Interrupt_Pending 0x1U
#define U0IIR_Interrupt_Pending_BIT 0
#define U0IIR_Interrupt_Identification_MASK 0xEU
#define U0IIR_Interrupt_Identification_BIT 1
#define U0IIR_FIFO_Enable_MASK 0xC0U
#define U0IIR_FIFO_Enable_BIT 6

#define U0FCR (*(volatile unsigned char *)0xE000C008)
#define U0FCR_FIFO_Enable_MASK 0x1U
#define U0FCR_FIFO_Enable 0x1U
#define U0FCR_FIFO_Enable_BIT 0
#define U0FCR_Rx_FIFO_Reset_MASK 0x2U
#define U0FCR_Rx_FIFO_Reset 0x2U
#define U0FCR_Rx_FIFO_Reset_BIT 1
#define U0FCR_Tx_FIFO_Reset_MASK 0x4U
#define U0FCR_Tx_FIFO_Reset 0x4U
#define U0FCR_Tx_FIFO_Reset_BIT 2
#define U0FCR_Rx_Trigger_Level_Select_MASK 0xC0U
#define U0FCR_Rx_Trigger_Level_Select_BIT 6

#define U0LCR (*(volatile unsigned char *)0xE000C00C)
#define U0LCR_Word_Length_Select_MASK 0x3U
#define U0LCR_Word_Length_Select_BIT 0
#define U0LCR_Stop_Bit_Select_MASK 0x4U
#define U0LCR_Stop_Bit_Select 0x4U
#define U0LCR_Stop_Bit_Select_BIT 2
#define U0LCR_Parity_Enable_MASK 0x8U
#define U0LCR_Parity_Enable 0x8U
#define U0LCR_Parity_Enable_BIT 3
#define U0LCR_Parity_Select_MASK 0x30U
#define U0LCR_Parity_Select_BIT 4
#define U0LCR_Break_Control_MASK 0x40U
#define U0LCR_Break_Control 0x40U
#define U0LCR_Break_Control_BIT 6
#define U0LCR_Divisor_Latch_Access_Bit_MASK 0x80U
#define U0LCR_Divisor_Latch_Access_Bit 0x80U
#define U0LCR_Divisor_Latch_Access_Bit_BIT 7

#define U0LSR (*(volatile unsigned char *)0xE000C014)
#define U0LSR_RDR_MASK 0x1U
#define U0LSR_RDR 0x1U
#define U0LSR_RDR_BIT 0
#define U0LSR_OE_MASK 0x2U
#define U0LSR_OE 0x2U
#define U0LSR_OE_BIT 1
#define U0LSR_PE_MASK 0x4U
#define U0LSR_PE 0x4U
#define U0LSR_PE_BIT 2
#define U0LSR_FE_MASK 0x8U
#define U0LSR_FE 0x8U
#define U0LSR_FE_BIT 3
#define U0LSR_BI_MASK 0x10U
#define U0LSR_BI 0x10U
#define U0LSR_BI_BIT 4
#define U0LSR_THRE_MASK 0x20U
#define U0LSR_THRE 0x20U
#define U0LSR_THRE_BIT 5
#define U0LSR_TEMT_MASK 0x40U
#define U0LSR_TEMT 0x40U
#define U0LSR_TEMT_BIT 6
#define U0LSR_RXFE_MASK 0x80U
#define U0LSR_RXFE 0x80U
#define U0LSR_RXFE_BIT 7

#define U0SCR (*(volatile unsigned char *)0xE000C01C)

#define U1RBR (*(volatile unsigned char *)0xE0010000)

#define U1THR (*(volatile unsigned char *)0xE0010000)

#define U1DLL (*(volatile unsigned char *)0xE0010000)

#define U1DLM (*(volatile unsigned char *)0xE0010004)

#define U1IER (*(volatile unsigned char *)0xE0010004)
#define U1IER_RBR_Interrupt_Enable_MASK 0x1U
#define U1IER_RBR_Interrupt_Enable 0x1U
#define U1IER_RBR_Interrupt_Enable_BIT 0
#define U1IER_THRE_Interrupt_Enable_MASK 0x2U
#define U1IER_THRE_Interrupt_Enable 0x2U
#define U1IER_THRE_Interrupt_Enable_BIT 1
#define U1IER_Rx_Line_Status_Interrupt_Enable_MASK 0x4U
#define U1IER_Rx_Line_Status_Interrupt_Enable 0x4U
#define U1IER_Rx_Line_Status_Interrupt_Enable_BIT 2
#define U1IER_Modem_Status_Interrupt_Enable_MASK 0x4U
#define U1IER_Modem_Status_Interrupt_Enable 0x4U
#define U1IER_Modem_Status_Interrupt_Enable_BIT 2

#define U1IIR (*(volatile unsigned char *)0xE0010008)
#define U1IIR_Interrupt_Pending_MASK 0x1U
#define U1IIR_Interrupt_Pending 0x1U
#define U1IIR_Interrupt_Pending_BIT 0
#define U1IIR_Interrupt_Identification_MASK 0xEU
#define U1IIR_Interrupt_Identification_BIT 1
#define U1IIR_FIFO_Enable_MASK 0xC0U
#define U1IIR_FIFO_Enable_BIT 6

#define U1FCR (*(volatile unsigned char *)0xE0010008)
#define U1FCR_FIFO_Enable_MASK 0x1U
#define U1FCR_FIFO_Enable 0x1U
#define U1FCR_FIFO_Enable_BIT 0
#define U1FCR_Rx_FIFO_Reset_MASK 0x2U
#define U1FCR_Rx_FIFO_Reset 0x2U
#define U1FCR_Rx_FIFO_Reset_BIT 1
#define U1FCR_Tx_FIFO_Reset_MASK 0x4U
#define U1FCR_Tx_FIFO_Reset 0x4U
#define U1FCR_Tx_FIFO_Reset_BIT 2
#define U1FCR_Rx_Trigger_Level_Select_MASK 0xC0U
#define U1FCR_Rx_Trigger_Level_Select_BIT 6

#define U1LCR (*(volatile unsigned char *)0xE001000C)
#define U1LCR_Word_Length_Select_MASK 0x3U
#define U1LCR_Word_Length_Select_BIT 0
#define U1LCR_Stop_Bit_Select_MASK 0x4U
#define U1LCR_Stop_Bit_Select 0x4U
#define U1LCR_Stop_Bit_Select_BIT 2
#define U1LCR_Parity_Enable_MASK 0x8U
#define U1LCR_Parity_Enable 0x8U
#define U1LCR_Parity_Enable_BIT 3
#define U1LCR_Parity_Select_MASK 0x30U
#define U1LCR_Parity_Select_BIT 4
#define U1LCR_Break_Control_MASK 0x40U
#define U1LCR_Break_Control 0x40U
#define U1LCR_Break_Control_BIT 6
#define U1LCR_Divisor_Latch_Access_Bit_MASK 0x80U
#define U1LCR_Divisor_Latch_Access_Bit 0x80U
#define U1LCR_Divisor_Latch_Access_Bit_BIT 7

#define U1MCR (*(volatile unsigned char *)0xE0010010)
#define U1MCR_DTR_Control_MASK 0x1U
#define U1MCR_DTR_Control 0x1U
#define U1MCR_DTR_Control_BIT 0
#define U1MCR_RTS_Control_MASK 0x2U
#define U1MCR_RTS_Control 0x2U
#define U1MCR_RTS_Control_BIT 1
#define U1MCR_Loopback_Mode_Select_MASK 0x10U
#define U1MCR_Loopback_Mode_Select 0x10U
#define U1MCR_Loopback_Mode_Select_BIT 4

#define U1LSR (*(volatile unsigned char *)0xE0010014)
#define U1LSR_RDR_MASK 0x1U
#define U1LSR_RDR 0x1U
#define U1LSR_RDR_BIT 0
#define U1LSR_OE_MASK 0x2U
#define U1LSR_OE 0x2U
#define U1LSR_OE_BIT 1
#define U1LSR_PE_MASK 0x4U
#define U1LSR_PE 0x4U
#define U1LSR_PE_BIT 2
#define U1LSR_FE_MASK 0x8U
#define U1LSR_FE 0x8U
#define U1LSR_FE_BIT 3
#define U1LSR_BI_MASK 0x10U
#define U1LSR_BI 0x10U
#define U1LSR_BI_BIT 4
#define U1LSR_THRE_MASK 0x20U
#define U1LSR_THRE 0x20U
#define U1LSR_THRE_BIT 5
#define U1LSR_TEMT_MASK 0x40U
#define U1LSR_TEMT 0x40U
#define U1LSR_TEMT_BIT 6
#define U1LSR_RXFE_MASK 0x80U
#define U1LSR_RXFE 0x80U
#define U1LSR_RXFE_BIT 7

#define U1MSR (*(volatile unsigned char *)0xE0010018)
#define U1MSR_Delta_CTS_MASK 0x1U
#define U1MSR_Delta_CTS 0x1U
#define U1MSR_Delta_CTS_BIT 0
#define U1MSR_Delta_DSR_MASK 0x2U
#define U1MSR_Delta_DSR 0x2U
#define U1MSR_Delta_DSR_BIT 1
#define U1MSR_Trailing_Edge_RI_MASK 0x4U
#define U1MSR_Trailing_Edge_RI 0x4U
#define U1MSR_Trailing_Edge_RI_BIT 2
#define U1MSR_Delta_DCD_MASK 0x8U
#define U1MSR_Delta_DCD 0x8U
#define U1MSR_Delta_DCD_BIT 3
#define U1MSR_CTS_MASK 0x10U
#define U1MSR_CTS 0x10U
#define U1MSR_CTS_BIT 4
#define U1MSR_DSR_MASK 0x20U
#define U1MSR_DSR 0x20U
#define U1MSR_DSR_BIT 5
#define U1MSR_RI_MASK 0x40U
#define U1MSR_RI 0x40U
#define U1MSR_RI_BIT 6
#define U1MSR_DCD_MASK 0x80U
#define U1MSR_DCD 0x80U
#define U1MSR_DCD_BIT 7

#define U1SCR (*(volatile unsigned char *)0xE001001C)

#define PWMIR (*(volatile unsigned long *)0xE0014000)
#define PWMIR_PWMMR0_Interrupt_MASK 0x1U
#define PWMIR_PWMMR0_Interrupt 0x1U
#define PWMIR_PWMMR0_Interrupt_BIT 0
#define PWMIR_PWMMR1_Interrupt_MASK 0x2U
#define PWMIR_PWMMR1_Interrupt 0x2U
#define PWMIR_PWMMR1_Interrupt_BIT 1
#define PWMIR_PWMMR2_Interrupt_MASK 0x4U
#define PWMIR_PWMMR2_Interrupt 0x4U
#define PWMIR_PWMMR2_Interrupt_BIT 2
#define PWMIR_PWMMR3_Interrupt_MASK 0x8U
#define PWMIR_PWMMR3_Interrupt 0x8U
#define PWMIR_PWMMR3_Interrupt_BIT 3
#define PWMIR_PWMMR4_Interrupt_MASK 0x10U
#define PWMIR_PWMMR4_Interrupt 0x10U
#define PWMIR_PWMMR4_Interrupt_BIT 4
#define PWMIR_PWMMR5_Interrupt_MASK 0x20U
#define PWMIR_PWMMR5_Interrupt 0x20U
#define PWMIR_PWMMR5_Interrupt_BIT 5
#define PWMIR_PWMMR6_Interrupt_MASK 0x40U
#define PWMIR_PWMMR6_Interrupt 0x40U
#define PWMIR_PWMMR6_Interrupt_BIT 6

#define PWMTCR (*(volatile unsigned long *)0xE0014004)
#define PWMTCR_Counter_Enable_MASK 0x1U
#define PWMTCR_Counter_Enable 0x1U
#define PWMTCR_Counter_Enable_BIT 0
#define PWMTCR_Counter_Reset_MASK 0x2U
#define PWMTCR_Counter_Reset 0x2U
#define PWMTCR_Counter_Reset_BIT 1
#define PWMTCR_PWM_Enable_MASK 0x8U
#define PWMTCR_PWM_Enable 0x8U
#define PWMTCR_PWM_Enable_BIT 3

#define PWMTC (*(volatile unsigned long *)0xE0014008)

#define PWMPR (*(volatile unsigned long *)0xE001400C)

#define PWMPC (*(volatile unsigned long *)0xE0014010)

#define PWMMCR (*(volatile unsigned long *)0xE0014014)
#define PWMMCR_Interrupt_on_PWMMR0_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR0 0x1U
#define PWMMCR_Interrupt_on_PWMMR0_BIT 0
#define PWMMCR_Reset_on_PWMMR0_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR0 0x1U
#define PWMMCR_Reset_on_PWMMR0_BIT 0
#define PWMMCR_Stop_on_PWMMR0_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR0 0x1U
#define PWMMCR_Stop_on_PWMMR0_BIT 0
#define PWMMCR_Interrupt_on_PWMMR1_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR1 0x1U
#define PWMMCR_Interrupt_on_PWMMR1_BIT 0
#define PWMMCR_Reset_on_PWMMR1_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR1 0x1U
#define PWMMCR_Reset_on_PWMMR1_BIT 0
#define PWMMCR_Stop_on_PWMMR1_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR1 0x1U
#define PWMMCR_Stop_on_PWMMR1_BIT 0
#define PWMMCR_Interrupt_on_PWMMR2_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR2 0x1U
#define PWMMCR_Interrupt_on_PWMMR2_BIT 0
#define PWMMCR_Reset_on_PWMMR2_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR2 0x1U
#define PWMMCR_Reset_on_PWMMR2_BIT 0
#define PWMMCR_Stop_on_PWMMR2_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR2 0x1U
#define PWMMCR_Stop_on_PWMMR2_BIT 0
#define PWMMCR_Interrupt_on_PWMMR3_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR3 0x1U
#define PWMMCR_Interrupt_on_PWMMR3_BIT 0
#define PWMMCR_Reset_on_PWMMR3_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR3 0x1U
#define PWMMCR_Reset_on_PWMMR3_BIT 0
#define PWMMCR_Stop_on_PWMMR3_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR3 0x1U
#define PWMMCR_Stop_on_PWMMR3_BIT 0
#define PWMMCR_Interrupt_on_PWMMR4_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR4 0x1U
#define PWMMCR_Interrupt_on_PWMMR4_BIT 0
#define PWMMCR_Reset_on_PWMMR4_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR4 0x1U
#define PWMMCR_Reset_on_PWMMR4_BIT 0
#define PWMMCR_Stop_on_PWMMR4_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR4 0x1U
#define PWMMCR_Stop_on_PWMMR4_BIT 0
#define PWMMCR_Interrupt_on_PWMMR5_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR5 0x1U
#define PWMMCR_Interrupt_on_PWMMR5_BIT 0
#define PWMMCR_Reset_on_PWMMR5_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR5 0x1U
#define PWMMCR_Reset_on_PWMMR5_BIT 0
#define PWMMCR_Stop_on_PWMMR5_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR5 0x1U
#define PWMMCR_Stop_on_PWMMR5_BIT 0
#define PWMMCR_Interrupt_on_PWMMR6_MASK 0x1U
#define PWMMCR_Interrupt_on_PWMMR6 0x1U
#define PWMMCR_Interrupt_on_PWMMR6_BIT 0
#define PWMMCR_Reset_on_PWMMR6_MASK 0x1U
#define PWMMCR_Reset_on_PWMMR6 0x1U
#define PWMMCR_Reset_on_PWMMR6_BIT 0
#define PWMMCR_Stop_on_PWMMR6_MASK 0x1U
#define PWMMCR_Stop_on_PWMMR6 0x1U
#define PWMMCR_Stop_on_PWMMR6_BIT 0

#define PWMMR0 (*(volatile unsigned long *)0xE0014018)

#define PWMMR1 (*(volatile unsigned long *)0xE001401C)

#define PWMMR2 (*(volatile unsigned long *)0xE0014020)

#define PWMMR3 (*(volatile unsigned long *)0xE0014024)

#define PWMMR4 (*(volatile unsigned long *)0xE0014040)

#define PWMMR5 (*(volatile unsigned long *)0xE0014044)

#define PWMMR6 (*(volatile unsigned long *)0xE0014048)

#define PWMPCR (*(volatile unsigned long *)0xE001404C)
#define PWMPCR_PWMSEL2_MASK 0x4U
#define PWMPCR_PWMSEL2 0x4U
#define PWMPCR_PWMSEL2_BIT 2
#define PWMPCR_PWMSEL3_MASK 0x8U
#define PWMPCR_PWMSEL3 0x8U
#define PWMPCR_PWMSEL3_BIT 3
#define PWMPCR_PWMSEL4_MASK 0x10U
#define PWMPCR_PWMSEL4 0x10U
#define PWMPCR_PWMSEL4_BIT 4
#define PWMPCR_PWMSEL5_MASK 0x20U
#define PWMPCR_PWMSEL5 0x20U
#define PWMPCR_PWMSEL5_BIT 5
#define PWMPCR_PWMSEL6_MASK 0x40U
#define PWMPCR_PWMSEL6 0x40U
#define PWMPCR_PWMSEL6_BIT 6
#define PWMPCR_PWMENA1_MASK 0x200U
#define PWMPCR_PWMENA1 0x200U
#define PWMPCR_PWMENA1_BIT 9
#define PWMPCR_PWMENA2_MASK 0x400U
#define PWMPCR_PWMENA2 0x400U
#define PWMPCR_PWMENA2_BIT 10
#define PWMPCR_PWMENA3_MASK 0x800U
#define PWMPCR_PWMENA3 0x800U
#define PWMPCR_PWMENA3_BIT 11
#define PWMPCR_PWMENA4_MASK 0x1000U
#define PWMPCR_PWMENA4 0x1000U
#define PWMPCR_PWMENA4_BIT 12
#define PWMPCR_PWMENA5_MASK 0x2000U
#define PWMPCR_PWMENA5 0x2000U
#define PWMPCR_PWMENA5_BIT 13
#define PWMPCR_PWMENA6_MASK 0x4000U
#define PWMPCR_PWMENA6 0x4000U
#define PWMPCR_PWMENA6_BIT 14

#define PWMLER (*(volatile unsigned long *)0xE0014050)
#define PWMLER_Enable_PWM_Match_0_Latch_MASK 0x1U
#define PWMLER_Enable_PWM_Match_0_Latch 0x1U
#define PWMLER_Enable_PWM_Match_0_Latch_BIT 0
#define PWMLER_Enable_PWM_Match_1_Latch_MASK 0x2U
#define PWMLER_Enable_PWM_Match_1_Latch 0x2U
#define PWMLER_Enable_PWM_Match_1_Latch_BIT 1
#define PWMLER_Enable_PWM_Match_2_Latch_MASK 0x4U
#define PWMLER_Enable_PWM_Match_2_Latch 0x4U
#define PWMLER_Enable_PWM_Match_2_Latch_BIT 2
#define PWMLER_Enable_PWM_Match_3_Latch_MASK 0x8U
#define PWMLER_Enable_PWM_Match_3_Latch 0x8U
#define PWMLER_Enable_PWM_Match_3_Latch_BIT 3
#define PWMLER_Enable_PWM_Match_4_Latch_MASK 0x10U
#define PWMLER_Enable_PWM_Match_4_Latch 0x10U
#define PWMLER_Enable_PWM_Match_4_Latch_BIT 4
#define PWMLER_Enable_PWM_Match_5_Latch_MASK 0x20U
#define PWMLER_Enable_PWM_Match_5_Latch 0x20U
#define PWMLER_Enable_PWM_Match_5_Latch_BIT 5
#define PWMLER_Enable_PWM_Match_6_Latch_MASK 0x40U
#define PWMLER_Enable_PWM_Match_6_Latch 0x40U
#define PWMLER_Enable_PWM_Match_6_Latch_BIT 6

#define I2C0CONSET (*(volatile unsigned char *)0xE001C000)
#define I2C0CONSET_AA_MASK 0x4U
#define I2C0CONSET_AA 0x4U
#define I2C0CONSET_AA_BIT 2
#define I2C0CONSET_SI_MASK 0x8U
#define I2C0CONSET_SI 0x8U
#define I2C0CONSET_SI_BIT 3
#define I2C0CONSET_STO_MASK 0x10U
#define I2C0CONSET_STO 0x10U
#define I2C0CONSET_STO_BIT 4
#define I2C0CONSET_STA_MASK 0x20U
#define I2C0CONSET_STA 0x20U
#define I2C0CONSET_STA_BIT 5
#define I2C0CONSET_I2EN_MASK 0x40U
#define I2C0CONSET_I2EN 0x40U
#define I2C0CONSET_I2EN_BIT 6

#define I2C0STAT (*(volatile unsigned char *)0xE001C004)

#define I2C0DAT (*(volatile unsigned char *)0xE001C008)

#define I2C0ADR (*(volatile unsigned char *)0xE001C00C)
#define I2C0ADR_GC_MASK 0x1U
#define I2C0ADR_GC 0x1U
#define I2C0ADR_GC_BIT 0
#define I2C0ADR_Address_MASK 0x7EU
#define I2C0ADR_Address_BIT 1

#define I2C0SCLH (*(volatile unsigned short *)0xE001C010)

#define I2C0SCLL (*(volatile unsigned short *)0xE001C014)

#define I2C0CONCLR (*(volatile unsigned char *)0xE001C018)
#define I2C0CONCLR_AAC_MASK 0x4U
#define I2C0CONCLR_AAC 0x4U
#define I2C0CONCLR_AAC_BIT 2
#define I2C0CONCLR_SIC_MASK 0x8U
#define I2C0CONCLR_SIC 0x8U
#define I2C0CONCLR_SIC_BIT 3
#define I2C0CONCLR_STAC_MASK 0x20U
#define I2C0CONCLR_STAC 0x20U
#define I2C0CONCLR_STAC_BIT 5
#define I2C0CONCLR_I2ENC_MASK 0x40U
#define I2C0CONCLR_I2ENC 0x40U
#define I2C0CONCLR_I2ENC_BIT 6

#define I2C1CONSET (*(volatile unsigned char *)0xE005C000)
#define I2C1CONSET_AA_MASK 0x4U
#define I2C1CONSET_AA 0x4U
#define I2C1CONSET_AA_BIT 2
#define I2C1CONSET_SI_MASK 0x8U
#define I2C1CONSET_SI 0x8U
#define I2C1CONSET_SI_BIT 3
#define I2C1CONSET_STO_MASK 0x10U
#define I2C1CONSET_STO 0x10U
#define I2C1CONSET_STO_BIT 4
#define I2C1CONSET_STA_MASK 0x20U
#define I2C1CONSET_STA 0x20U
#define I2C1CONSET_STA_BIT 5
#define I2C1CONSET_I2EN_MASK 0x40U
#define I2C1CONSET_I2EN 0x40U
#define I2C1CONSET_I2EN_BIT 6

#define I2C1STAT (*(volatile unsigned char *)0xE005C004)

#define I2C1DAT (*(volatile unsigned char *)0xE005C008)

#define I2C1ADR (*(volatile unsigned char *)0xE005C00C)
#define I2C1ADR_GC_MASK 0x1U
#define I2C1ADR_GC 0x1U
#define I2C1ADR_GC_BIT 0
#define I2C1ADR_Address_MASK 0x7EU
#define I2C1ADR_Address_BIT 1

#define I2C1SCLH (*(volatile unsigned short *)0xE005C010)

#define I2C1SCLL (*(volatile unsigned short *)0xE005C014)

#define I2C1CONCLR (*(volatile unsigned char *)0xE005C018)
#define I2C1CONCLR_AAC_MASK 0x4U
#define I2C1CONCLR_AAC 0x4U
#define I2C1CONCLR_AAC_BIT 2
#define I2C1CONCLR_SIC_MASK 0x8U
#define I2C1CONCLR_SIC 0x8U
#define I2C1CONCLR_SIC_BIT 3
#define I2C1CONCLR_STAC_MASK 0x20U
#define I2C1CONCLR_STAC 0x20U
#define I2C1CONCLR_STAC_BIT 5
#define I2C1CONCLR_I2ENC_MASK 0x40U
#define I2C1CONCLR_I2ENC 0x40U
#define I2C1CONCLR_I2ENC_BIT 6

#define SPCR (*(volatile unsigned char *)0xE0020000)
#define SPCR_CPHA_MASK 0x8U
#define SPCR_CPHA 0x8U
#define SPCR_CPHA_BIT 3
#define SPCR_CPOL_MASK 0x10U
#define SPCR_CPOL 0x10U
#define SPCR_CPOL_BIT 4
#define SPCR_MSTR_MASK 0x20U
#define SPCR_MSTR 0x20U
#define SPCR_MSTR_BIT 5
#define SPCR_LSBF_MASK 0x40U
#define SPCR_LSBF 0x40U
#define SPCR_LSBF_BIT 6
#define SPCR_SPIE_MASK 0x80U
#define SPCR_SPIE 0x80U
#define SPCR_SPIE_BIT 7

#define SPSR (*(volatile unsigned char *)0xE0020004)
#define SPSR_ABRT_MASK 0x8U
#define SPSR_ABRT 0x8U
#define SPSR_ABRT_BIT 3
#define SPSR_MODF_MASK 0x10U
#define SPSR_MODF 0x10U
#define SPSR_MODF_BIT 4
#define SPSR_ROVR_MASK 0x20U
#define SPSR_ROVR 0x20U
#define SPSR_ROVR_BIT 5
#define SPSR_WCOL_MASK 0x40U
#define SPSR_WCOL 0x40U
#define SPSR_WCOL_BIT 6
#define SPSR_SPIF_MASK 0x80U
#define SPSR_SPIF 0x80U
#define SPSR_SPIF_BIT 7

#define SPDR (*(volatile unsigned char *)0xE0020008)

#define SPCCR (*(volatile unsigned char *)0xE002000C)

#define SPINT (*(volatile unsigned char *)0xE002001C)

#define ILR (*(volatile unsigned long *)0xE0024000)
#define ILR_RTCCIF_MASK 0x1U
#define ILR_RTCCIF 0x1U
#define ILR_RTCCIF_BIT 0
#define ILR_RTCALF_MASK 0x2U
#define ILR_RTCALF 0x2U
#define ILR_RTCALF_BIT 1

#define CTC (*(volatile unsigned long *)0xE0024004)
#define CTC_Clock_Tick_Counter_MASK 0xFFFEU
#define CTC_Clock_Tick_Counter_BIT 1

#define CCR (*(volatile unsigned long *)0xE0024008)
#define CCR_CLKEN_MASK 0x1U
#define CCR_CLKEN 0x1U
#define CCR_CLKEN_BIT 0
#define CCR_CTCRST_MASK 0x2U
#define CCR_CTCRST 0x2U
#define CCR_CTCRST_BIT 1
#define CCR_CTTEST_MASK 0xCU
#define CCR_CTTEST_BIT 2

#define CIIR (*(volatile unsigned long *)0xE002400C)

#define AMR (*(volatile unsigned long *)0xE0024010)
#define AMR_AMRSEC_MASK 0x1U
#define AMR_AMRSEC 0x1U
#define AMR_AMRSEC_BIT 0
#define AMR_AMRMIN_MASK 0x2U
#define AMR_AMRMIN 0x2U
#define AMR_AMRMIN_BIT 1
#define AMR_AMRHOUR_MASK 0x4U
#define AMR_AMRHOUR 0x4U
#define AMR_AMRHOUR_BIT 2
#define AMR_AMRDOM_MASK 0x8U
#define AMR_AMRDOM 0x8U
#define AMR_AMRDOM_BIT 3
#define AMR_AMRDOW_MASK 0x10U
#define AMR_AMRDOW 0x10U
#define AMR_AMRDOW_BIT 4
#define AMR_AMRDOY_MASK 0x20U
#define AMR_AMRDOY 0x20U
#define AMR_AMRDOY_BIT 5
#define AMR_AMRMON_MASK 0x40U
#define AMR_AMRMON 0x40U
#define AMR_AMRMON_BIT 6
#define AMR_AMRYEAR_MASK 0x80U
#define AMR_AMRYEAR 0x80U
#define AMR_AMRYEAR_BIT 7

#define CTIME0 (*(volatile unsigned long *)0xE0024014)
#define CTIME0_Seconds_MASK 0x3FU
#define CTIME0_Seconds_BIT 0
#define CTIME0_Minutes_MASK 0x3F00U
#define CTIME0_Minutes_BIT 8
#define CTIME0_Hours_MASK 0x1F0000U
#define CTIME0_Hours_BIT 16
#define CTIME0_Day_of_Week_MASK 0xF8000000U
#define CTIME0_Day_of_Week_BIT 27

#define CTIME1 (*(volatile unsigned long *)0xE0024018)
#define CTIME1_Day_of_Month_MASK 0x1FU
#define CTIME1_Day_of_Month_BIT 0
#define CTIME1_Month_MASK 0xF00U
#define CTIME1_Month_BIT 8
#define CTIME1_Year_MASK 0xFFF0000U
#define CTIME1_Year_BIT 16

#define CTIME2 (*(volatile unsigned long *)0xE002401C)
#define CTIME2_Day_of_Year_MASK 0xFFFU
#define CTIME2_Day_of_Year_BIT 0

#define SEC (*(volatile unsigned long *)0xE0024020)

#define MIN (*(volatile unsigned long *)0xE0024024)

#define HOUR (*(volatile unsigned long *)0xE0024028)

#define DOM (*(volatile unsigned long *)0xE002402C)

#define DOW (*(volatile unsigned long *)0xE0024030)

#define DOY (*(volatile unsigned long *)0xE0024034)

#define MONTH (*(volatile unsigned long *)0xE0024038)

#define YEAR (*(volatile unsigned long *)0xE002403C)

#define ALSEC (*(volatile unsigned long *)0xE0024060)

#define ALMIN (*(volatile unsigned long *)0xE0024064)

#define ALHOUR (*(volatile unsigned long *)0xE0024068)

#define ALDOM (*(volatile unsigned long *)0xE002406C)

#define ALDOW (*(volatile unsigned long *)0xE0024070)

#define ALDOY (*(volatile unsigned long *)0xE0024074)

#define ALMON (*(volatile unsigned long *)0xE0024078)

#define ALYEAR (*(volatile unsigned long *)0xE002407C)

#define PREINT (*(volatile unsigned long *)0xE0024080)

#define PREFRAC (*(volatile unsigned long *)0xE0024084)

#define IO0PIN (*(volatile unsigned long *)0xE0028000)

#define IO0SET (*(volatile unsigned long *)0xE0028004)

#define IO0DIR (*(volatile unsigned long *)0xE0028008)

#define IO0CLR (*(volatile unsigned long *)0xE002800C)

#define IO1PIN (*(volatile unsigned long *)0xE0028010)

#define IO1SET (*(volatile unsigned long *)0xE0028014)

#define IO1DIR (*(volatile unsigned long *)0xE0028018)

#define IO1CLR (*(volatile unsigned long *)0xE002801C)

#define PINSEL0 (*(volatile unsigned long *)0xE002C000)

#define PINSEL1 (*(volatile unsigned long *)0xE002C004)

#define PINSEL2 (*(volatile unsigned long *)0xE002C014)

#define AD0CR (*(volatile unsigned long *)0xE0034000)
#define AD0CR_SEL_MASK 0xFFU
#define AD0CR_SEL_BIT 0
#define AD0CR_CLKDIV_MASK 0xFF00U
#define AD0CR_CLKDIV_BIT 8
#define AD0CR_BURST_MASK 0x10000U
#define AD0CR_BURST 0x10000U
#define AD0CR_BURST_BIT 16
#define AD0CR_CLKS_MASK 0xE0000U
#define AD0CR_CLKS_BIT 17
#define AD0CR_PDN_MASK 0x200000U
#define AD0CR_PDN 0x200000U
#define AD0CR_PDN_BIT 21
#define AD0CR_TEST1_0_MASK 0xC00000U
#define AD0CR_TEST1_0_BIT 22
#define AD0CR_START_MASK 0x7000000U
#define AD0CR_START_BIT 24
#define AD0CR_EDGE_MASK 0x8000000U
#define AD0CR_EDGE 0x8000000U
#define AD0CR_EDGE_BIT 27

#define AD0DR (*(volatile unsigned long *)0xE0034004)
#define AD0DR_VddA_MASK 0xFFC0U
#define AD0DR_VddA_BIT 6
#define AD0DR_CHN_MASK 0x7000000U
#define AD0DR_CHN_BIT 24
#define AD0DR_OVERUN_MASK 0x40000000U
#define AD0DR_OVERUN 0x40000000U
#define AD0DR_OVERUN_BIT 30
#define AD0DR_DONE_MASK 0x80000000U
#define AD0DR_DONE 0x80000000U
#define AD0DR_DONE_BIT 31

#define ADGSR (*(volatile unsigned long *)0xE0034008)
#define ADGSR_BURST_MASK 0x10000U
#define ADGSR_BURST 0x10000U
#define ADGSR_BURST_BIT 16
#define ADGSR_START_MASK 0x7000000U
#define ADGSR_START_BIT 24
#define ADGSR_EDGE_MASK 0x8000000U
#define ADGSR_EDGE 0x8000000U
#define ADGSR_EDGE_BIT 27

#define AD1CR (*(volatile unsigned long *)0xE0060000)
#define AD1CR_SEL_MASK 0xFFU
#define AD1CR_SEL_BIT 0
#define AD1CR_CLKDIV_MASK 0xFF00U
#define AD1CR_CLKDIV_BIT 8
#define AD1CR_BURST_MASK 0x10000U
#define AD1CR_BURST 0x10000U
#define AD1CR_BURST_BIT 16
#define AD1CR_CLKS_MASK 0xE0000U
#define AD1CR_CLKS_BIT 17
#define AD1CR_PDN_MASK 0x200000U
#define AD1CR_PDN 0x200000U
#define AD1CR_PDN_BIT 21
#define AD1CR_TEST1_0_MASK 0xC00000U
#define AD1CR_TEST1_0_BIT 22
#define AD1CR_START_MASK 0x7000000U
#define AD1CR_START_BIT 24
#define AD1CR_EDGE_MASK 0x8000000U
#define AD1CR_EDGE 0x8000000U
#define AD1CR_EDGE_BIT 27

#define AD1DR (*(volatile unsigned long *)0xE0060004)
#define AD1DR_VddA_MASK 0xFFC0U
#define AD1DR_VddA_BIT 6
#define AD1DR_CHN_MASK 0x7000000U
#define AD1DR_CHN_BIT 24
#define AD1DR_OVERUN_MASK 0x40000000U
#define AD1DR_OVERUN 0x40000000U
#define AD1DR_OVERUN_BIT 30
#define AD1DR_DONE_MASK 0x80000000U
#define AD1DR_DONE 0x80000000U
#define AD1DR_DONE_BIT 31

#define DACR (*(volatile unsigned long *)0xE006C000)
#define DACR_VALUE_MASK 0xFFC0U
#define DACR_VALUE_BIT 6
#define DACR_BIAS_MASK 0x10000U
#define DACR_BIAS 0x10000U
#define DACR_BIAS_BIT 16

#define SSPCR0 (*(volatile unsigned long *)0xE0068000)
#define SSPCR0_SCR_MASK 0xFF00U
#define SSPCR0_SCR_BIT 8
#define SSPCR0_SPH_MASK 0x80U
#define SSPCR0_SPH 0x80U
#define SSPCR0_SPH_BIT 7
#define SSPCR0_SPO_MASK 0x40U
#define SSPCR0_SPO 0x40U
#define SSPCR0_SPO_BIT 6
#define SSPCR0_FRF_MASK 0x30U
#define SSPCR0_FRF_BIT 4
#define SSPCR0_DSS_MASK 0xFU
#define SSPCR0_DSS_BIT 0

#define SSPCR1 (*(volatile unsigned long *)0xE0068004)
#define SSPCR1_SOD_MASK 0x8U
#define SSPCR1_SOD 0x8U
#define SSPCR1_SOD_BIT 3
#define SSPCR1_MS_MASK 0x4U
#define SSPCR1_MS 0x4U
#define SSPCR1_MS_BIT 2
#define SSPCR1_SSE_MASK 0x2U
#define SSPCR1_SSE 0x2U
#define SSPCR1_SSE_BIT 1
#define SSPCR1_LBE_MASK 0x1U
#define SSPCR1_LBE 0x1U
#define SSPCR1_LBE_BIT 0

#define SSPDR (*(volatile unsigned long *)0xE0068008)

#define SSPSR (*(volatile unsigned long *)0xE006800C)
#define SSPSR_BSY_MASK 0x10U
#define SSPSR_BSY 0x10U
#define SSPSR_BSY_BIT 4
#define SSPSR_RFF_MASK 0x8U
#define SSPSR_RFF 0x8U
#define SSPSR_RFF_BIT 3
#define SSPSR_RNE_MASK 0x4U
#define SSPSR_RNE 0x4U
#define SSPSR_RNE_BIT 2
#define SSPSR_TNF_MASK 0x2U
#define SSPSR_TNF 0x2U
#define SSPSR_TNF_BIT 1
#define SSPSR_TFE_MASK 0x1U
#define SSPSR_TFE 0x1U
#define SSPSR_TFE_BIT 0

#define SSPCPSR (*(volatile unsigned long *)0xE0068010)

#define SSPIMSC (*(volatile unsigned long *)0xE0068014)
#define SSPIMSC_TXIM_MASK 0x8U
#define SSPIMSC_TXIM 0x8U
#define SSPIMSC_TXIM_BIT 3
#define SSPIMSC_RXIM_MASK 0x4U
#define SSPIMSC_RXIM 0x4U
#define SSPIMSC_RXIM_BIT 2
#define SSPIMSC_RTIM_MASK 0x2U
#define SSPIMSC_RTIM 0x2U
#define SSPIMSC_RTIM_BIT 1
#define SSPIMSC_RORIM_MASK 0x1U
#define SSPIMSC_RORIM 0x1U
#define SSPIMSC_RORIM_BIT 0

#define SSPRIS (*(volatile unsigned long *)0xE0068018)
#define SSPRIS_TXRIS_MASK 0x8U
#define SSPRIS_TXRIS 0x8U
#define SSPRIS_TXRIS_BIT 3
#define SSPRIS_RXRIS_MASK 0x4U
#define SSPRIS_RXRIS 0x4U
#define SSPRIS_RXRIS_BIT 2
#define SSPRIS_RTRIS_MASK 0x2U
#define SSPRIS_RTRIS 0x2U
#define SSPRIS_RTRIS_BIT 1
#define SSPRIS_RORRIS_MASK 0x1U
#define SSPRIS_RORRIS 0x1U
#define SSPRIS_RORRIS_BIT 0

#define SSPMIS (*(volatile unsigned long *)0xE006801C)
#define SSPMIS_TXMIS_MASK 0x8U
#define SSPMIS_TXMIS 0x8U
#define SSPMIS_TXMIS_BIT 3
#define SSPMIS_RXMIS_MASK 0x4U
#define SSPMIS_RXMIS 0x4U
#define SSPMIS_RXMIS_BIT 2
#define SSPMIS_RTMIS_MASK 0x2U
#define SSPMIS_RTMIS 0x2U
#define SSPMIS_RTMIS_BIT 1
#define SSPMIS_RORMIS_MASK 0x1U
#define SSPMIS_RORMIS 0x1U
#define SSPMIS_RORMIS_BIT 0

#define SSPICR (*(volatile unsigned long *)0xE0068020)
#define SSPICR_RTIC_MASK 0x2U
#define SSPICR_RTIC 0x2U
#define SSPICR_RTIC_BIT 1
#define SSPICR_RORIC_MASK 0x1U
#define SSPICR_RORIC 0x1U
#define SSPICR_RORIC_BIT 0

#define SSPPeriphID0_3 (*(volatile unsigned long *)0xE0068FE0)

#define SSPPCellID0_3 (*(volatile unsigned long *)0xE0068FFC)

#define MAMCR (*(volatile unsigned char *)0xE01FC000)
#define MAMCR_MAM_mode_control_MASK 0x3U
#define MAMCR_MAM_mode_control_BIT 0

#define MAMTIM (*(volatile unsigned char *)0xE01FC004)
#define MAMTIM_MAM_Fetch_Cycle_timing_MASK 0x7U
#define MAMTIM_MAM_Fetch_Cycle_timing_BIT 0

#define MEMMAP (*(volatile unsigned char *)0xE01FC040)
#define MEMMAP_MAP1_0_MASK 0x3U
#define MEMMAP_MAP1_0_BIT 0

#define PLLCON (*(volatile unsigned char *)0xE01FC080)
#define PLLCON_PLLE_MASK 0x1U
#define PLLCON_PLLE 0x1U
#define PLLCON_PLLE_BIT 0
#define PLLCON_PLLC_MASK 0x2U
#define PLLCON_PLLC 0x2U
#define PLLCON_PLLC_BIT 1

#define PLLCFG (*(volatile unsigned char *)0xE01FC084)
#define PLLCFG_MSEL4_0_MASK 0xFU
#define PLLCFG_MSEL4_0_BIT 0
#define PLLCFG_PSEL1_0_MASK 0x60U
#define PLLCFG_PSEL1_0_BIT 5

#define PLLSTAT (*(volatile unsigned short *)0xE01FC088)
#define PLLSTAT_MSEL4_0_MASK 0xFU
#define PLLSTAT_MSEL4_0_BIT 0
#define PLLSTAT_PSEL1_0_MASK 0x60U
#define PLLSTAT_PSEL1_0_BIT 5
#define PLLSTAT_PLLE_MASK 0x100U
#define PLLSTAT_PLLE 0x100U
#define PLLSTAT_PLLE_BIT 8
#define PLLSTAT_PLLC_MASK 0x200U
#define PLLSTAT_PLLC 0x200U
#define PLLSTAT_PLLC_BIT 9
#define PLLSTAT_PLOCK_MASK 0x400U
#define PLLSTAT_PLOCK 0x400U
#define PLLSTAT_PLOCK_BIT 10

#define PLLFEED (*(volatile unsigned char *)0xE01FC08C)

#define PCON (*(volatile unsigned char *)0xE01FC0C0)
#define PCON_IDL_MASK 0x1U
#define PCON_IDL 0x1U
#define PCON_IDL_BIT 0
#define PCON_PD_MASK 0x2U
#define PCON_PD 0x2U
#define PCON_PD_BIT 1

#define PCONP (*(volatile unsigned long *)0xE01FC0C4)
#define PCONP_PCTIM0_MASK 0x2U
#define PCONP_PCTIM0 0x2U
#define PCONP_PCTIM0_BIT 1
#define PCONP_PCTIM1_MASK 0x4U
#define PCONP_PCTIM1 0x4U
#define PCONP_PCTIM1_BIT 2
#define PCONP_PCURT0_MASK 0x8U
#define PCONP_PCURT0 0x8U
#define PCONP_PCURT0_BIT 3
#define PCONP_PCURT1_MASK 0x10U
#define PCONP_PCURT1 0x10U
#define PCONP_PCURT1_BIT 4
#define PCONP_PCPWM0_MASK 0x20U
#define PCONP_PCPWM0 0x20U
#define PCONP_PCPWM0_BIT 5
#define PCONP_PCI2C_MASK 0x80U
#define PCONP_PCI2C 0x80U
#define PCONP_PCI2C_BIT 7
#define PCONP_PCSPIO_MASK 0x100U
#define PCONP_PCSPIO 0x100U
#define PCONP_PCSPIO_BIT 8
#define PCONP_PCRTC_MASK 0x200U
#define PCONP_PCRTC 0x200U
#define PCONP_PCRTC_BIT 9
#define PCONP_PCSPI1_MASK 0x400U
#define PCONP_PCSPI1 0x400U
#define PCONP_PCSPI1_BIT 10
#define PCONP_PCAD_MASK 0x1000U
#define PCONP_PCAD 0x1000U
#define PCONP_PCAD_BIT 12

#define VPBDIV (*(volatile unsigned char *)0xE01FC100)
#define VPBDIV_VPBDIV_MASK 0x3U
#define VPBDIV_VPBDIV_BIT 0
#define VPBDIV_XCLKDIV_MASK 0x30U
#define VPBDIV_XCLKDIV_BIT 4

#define RSID (*(volatile unsigned char *)0xE01FC180)

#define CSPR (*(volatile unsigned char *)0xE01FC180)

#define EXTINT (*(volatile unsigned char *)0xE01FC140)
#define EXTINT_EINT0_MASK 0x1U
#define EXTINT_EINT0 0x1U
#define EXTINT_EINT0_BIT 0
#define EXTINT_EINT1_MASK 0x2U
#define EXTINT_EINT1 0x2U
#define EXTINT_EINT1_BIT 1
#define EXTINT_EINT2_MASK 0x4U
#define EXTINT_EINT2 0x4U
#define EXTINT_EINT2_BIT 2
#define EXTINT_EINT3_MASK 0x8U
#define EXTINT_EINT3 0x8U
#define EXTINT_EINT3_BIT 3

#define INTWAKE (*(volatile unsigned char *)0xE01FC144)

#define EXTMODE (*(volatile unsigned char *)0xE01FC148)

#define EXTPOLAR (*(volatile unsigned char *)0xE01FC14C)

#define VICIRQStatus (*(volatile unsigned long *)0xFFFFF000)

#define VICFIQStatus (*(volatile unsigned long *)0xFFFFF004)

#define VICRawIntr (*(volatile unsigned long *)0xFFFFF008)

#define VICIntSelect (*(volatile unsigned long *)0xFFFFF00C)

#define VICIntEnable (*(volatile unsigned long *)0xFFFFF010)

#define VICIntEnClr (*(volatile unsigned long *)0xFFFFF014)

#define VICSoftInt (*(volatile unsigned long *)0xFFFFF018)

#define VICSoftIntClear (*(volatile unsigned long *)0xFFFFF01C)

#define VICProtection (*(volatile unsigned long *)0xFFFFF020)

#define VICVectAddr (*(volatile unsigned long *)0xFFFFF030)

#define VICDefVectAddr (*(volatile unsigned long *)0xFFFFF034)

#define VICVectAddr0 (*(volatile unsigned long *)0xFFFFF100)

#define VICVectAddr1 (*(volatile unsigned long *)0xFFFFF104)

#define VICVectAddr2 (*(volatile unsigned long *)0xFFFFF108)

#define VICVectAddr3 (*(volatile unsigned long *)0xFFFFF10C)

#define VICVectAddr4 (*(volatile unsigned long *)0xFFFFF110)

#define VICVectAddr5 (*(volatile unsigned long *)0xFFFFF114)

#define VICVectAddr6 (*(volatile unsigned long *)0xFFFFF118)

#define VICVectAddr7 (*(volatile unsigned long *)0xFFFFF11C)

#define VICVectAddr8 (*(volatile unsigned long *)0xFFFFF120)

#define VICVectAddr9 (*(volatile unsigned long *)0xFFFFF124)

#define VICVectAddr10 (*(volatile unsigned long *)0xFFFFF128)

#define VICVectAddr11 (*(volatile unsigned long *)0xFFFFF12C)

#define VICVectAddr12 (*(volatile unsigned long *)0xFFFFF130)

#define VICVectAddr13 (*(volatile unsigned long *)0xFFFFF134)

#define VICVectAddr14 (*(volatile unsigned long *)0xFFFFF138)

#define VICVectAddr15 (*(volatile unsigned long *)0xFFFFF13C)

#define VICVectCntl0 (*(volatile unsigned long *)0xFFFFF200)

#define VICVectCntl1 (*(volatile unsigned long *)0xFFFFF204)

#define VICVectCntl2 (*(volatile unsigned long *)0xFFFFF208)

#define VICVectCntl3 (*(volatile unsigned long *)0xFFFFF20C)

#define VICVectCntl4 (*(volatile unsigned long *)0xFFFFF210)

#define VICVectCntl5 (*(volatile unsigned long *)0xFFFFF214)

#define VICVectCntl6 (*(volatile unsigned long *)0xFFFFF218)

#define VICVectCntl7 (*(volatile unsigned long *)0xFFFFF21C)

#define VICVectCntl8 (*(volatile unsigned long *)0xFFFFF220)

#define VICVectCntl9 (*(volatile unsigned long *)0xFFFFF224)

#define VICVectCntl10 (*(volatile unsigned long *)0xFFFFF228)

#define VICVectCntl11 (*(volatile unsigned long *)0xFFFFF22C)

#define VICVectCntl12 (*(volatile unsigned long *)0xFFFFF230)

#define VICVectCntl13 (*(volatile unsigned long *)0xFFFFF234)

#define VICVectCntl14 (*(volatile unsigned long *)0xFFFFF238)

#define VICVectCntl15 (*(volatile unsigned long *)0xFFFFF23C)


#endif
