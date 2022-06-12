#pragma once

void privilege_escalate();
void privilege_drop();

#define ESCALATED(X) {privilege_escalate();X;privilege_drop();}

