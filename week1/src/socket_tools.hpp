#pragma once

struct addrinfo;

int CreateDgramSocket(const char* address, const char* port, addrinfo* res_addr);
