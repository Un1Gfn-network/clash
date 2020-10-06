#pragma once

typedef enum {
  DOWN,
  FAIL,
  UP
} Status;

void status_init();

void status_change_to(const Status s);

void status_change_from_to(const Status s0,const Status s1);

Status status_wait_change_from(const Status s0);
