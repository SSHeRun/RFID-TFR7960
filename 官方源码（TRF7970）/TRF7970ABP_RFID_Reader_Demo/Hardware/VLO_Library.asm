;*******************************************************************************
;   Code for application report - "VLO Library"
;*******************************************************************************
; THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
; REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
; INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
; FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
; COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
; TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
; POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
; INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
; YOUR USE OF THE PROGRAM.
;
; IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
; CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
; THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
; OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
; OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
; EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
; REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
; OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
; USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
; AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
; YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
; (U.S.$500).
;
; Unless otherwise stated, the Program written and copyrighted
; by Texas Instruments is distributed as "freeware".  You may,
; only under TI's copyright in the Program, use and modify the
; Program without any charge or restriction.  You may
; distribute to third parties, provided that you transfer a
; copy of this license to the third party and the third party
; agrees to these terms by its first use of the Program. You
; must reproduce the copyright notice and any other legend of
; ownership on each copy or partial copy, of the Program.
;
; You acknowledge and agree that the Program contains
; copyrighted material, trade secrets and other TI proprietary
; information and is protected by copyright laws,
; international copyright treaties, and trade secret laws, as
; well as other intellectual property laws.  To protect TI's
; rights in the Program, you agree not to decompile, reverse
; engineer, disassemble or otherwise translate any object code
; versions of the Program to a human-readable form.  You agree
; that in no event will you alter, remove or destroy any
; copyright notice included in the Program.  TI reserves all
; rights not specifically granted under this license. Except
; as specifically provided herein, nothing in this agreement
; shall be construed as conferring by implication, estoppel,
; or otherwise, upon you, any license or other right under any
; TI patents, copyrights or trade secrets.
;
; You may not use the Program in non-TI devices.
;
;*******************************************************************************
;   L. Westlund
;   Texas Instruments Inc.
;   March 2006
;   Built with Code Composer Essentials 2.0
;*******************************************************************************
 .cdecls C,LIST,    "msp430g2403.h"

            ;Functions
            .def      TI_measureVLO
            ;Variables
            .bss TI_8MHz_Counts_Per_VLO_Clock, 2
            .align 2
            .text

;DEVICE_TYPE .set 2                          ; 2xx devices with a Timer_A3
DEVICE_TYPE .set 3                          ; 2xx devices with a Timer_A2

 .if DEVICE_TYPE = 3                        ; For Timer_A2 devices
TACCTLX .set TACCTL0
TACCRX .set TACCR0
 .else
TACCTLX .set TACCTL2
TACCRX .set TACCR2
 .endif
 
;-------------------------------------------------------------------------------
TI_measureVLO
;           returns: r12
;           -An int representing the number of 8MHz clock pulses in one VLO cycle
;           -This value is identical to the number put into TI_8MHz_Counts_Per_VLO_Clock
;-------------------------------------------------------------------------------
            mov.b   &BCSCTL1,     r15       ; preserve previous settings
            mov.b   &DCOCTL,      r14
            push.b  &BCSCTL2
            push.b  &BCSCTL3
            push.b  &P2SEL
            bic.b   #0xC0,        &P2SEL    ; clear P2SEL bits to avoid XTAL interference.
            mov.b   &CALBC1_1MHZ, &BCSCTL1  ; Set range
            mov.b   &CALDCO_1MHZ, &DCOCTL   ; Set DCO step + modulation
            mov.w   #CM_1+CCIS_1+CAP,&TACCTLX ; CAP, ACLK
            mov.w   #TASSEL_2+MC_2+TACLR, &TACTL; SMCLK, cont-mode, clear
            mov.b   #LFXT1S_2,    &BCSCTL3  ; ACLK = VLO
            clr.b   &BCSCTL2
            bis.b   #DIVA_3,      &BCSCTL1  ; ACLK=VLO/8
            bic.w   #CCIFG,       &TACCTLX  ; Clear capture flag
edge_one    bit.w   #CCIFG,       &TACCTLX  ; Test capture flag to skip first signal
            jz      edge_one
            bic.w   #CCIFG,       &TACCTLX  ; Clear capture flag
edge_two    bit.w   #CCIFG,       &TACCTLX  ; Test capture flag to skip second signal
            jz      edge_two                ;
            mov.w   &TACCRX,      r13       ; save hardware captured value
            bic.w   #CCIFG,       &TACCTLX  ; Clear capture flag
edge_three  bit.w   #CCIFG,       &TACCTLX  ; Test capture flag to capture a good clock
            jz      edge_three              ;
            bic.w   #MC_3,        &TACTL    ; stop timer
            mov.w   &TACCRX,      r12
            sub.w   r13,          r12
            mov.w   r12,          &TI_8MHz_Counts_Per_VLO_Clock
            mov.b   r15,          &BCSCTL1
            mov.b   r14,          &DCOCTL
            pop.b   &P2SEL
            pop.b   &BCSCTL3
            pop.b   &BCSCTL2
            ret

            .end
