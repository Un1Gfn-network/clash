#pragma once

// Separate file descriptor, no need for ioctl_init()
void ioctl_tun_create(const char *__restrict const dev);
