/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <trace.h>

static iringbuf iringbufs[MAX_IRING_BUF] = {};
static iringbuf *head = NULL;
static iringbuf *tail = NULL;
static bool initFlag = false;

static void init_IRingBuf() {
    initFlag = true;
    /**
     * Recurring linked lists, in which a node needs to be occupied 
     * as a condition for determining when the queue is full.
     */
    int i;
    for (i=0; i<MAX_IRING_BUF; i++) {
        iringbufs[i].next = &iringbufs[(i+1)%MAX_IRING_BUF];
    }
    head = &iringbufs[0];
    tail = &iringbufs[0];
}

void insert_IRingBuf(char *p) {
    if (initFlag == false) init_IRingBuf();
    if (tail->next == head) {
        // iringbufs is full
        memset(head->buf, 0, sizeof(head->buf));
        head = head->next;
    }
    memcpy(tail->buf, p, 128);
    tail = tail->next;
}

void display_IRingBuf() {
    if (head == tail) {
        printf("The program hasn't started running yet.\n");
        return;
    }
    iringbuf *cur = head;
    while (cur->next != tail) {
        printf("      %s\n", cur->buf);
        cur = cur->next;
    }
    printf("  --> %s\n", cur->buf);
}
