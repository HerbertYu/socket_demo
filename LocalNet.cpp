//
// Created by Herbert on 2021/11/13.
//

#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifdef ANDROID
#include <linux/sysctl.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#else

#include <sys/sysctl.h>
#include <net/if_dl.h>

#endif

#include <net/if.h>

#include <cstdio>
#include <cstdlib>

#include "LocalNet.h"

#define BROADCAST_PORT 7260

namespace PianoLesson {

    LocalNet::LocalNet() {
        generate();
        generateMac();
    }

    const char *LocalNet::ipAddr() const {
        return _ipAddr.c_str();
    }

    const char *LocalNet::broadcastAddr() const {
        return _broadcastAddr.c_str();
    }

    const char *LocalNet::macAddr() const {
        return _macAddr.c_str();
    }

    int LocalNet::broadcastPort() const {
        return BROADCAST_PORT;
    }

    bool LocalNet::generate() {
        struct ifaddrs *ifaddr;
        int family;
        char host[NI_MAXHOST];
        struct sockaddr_in *addr;

        if (getifaddrs(&ifaddr) == -1) {
            fprintf(stderr, "[Message][LocalNet]getifaddrs error\n");
            return false;
        }

        for (auto ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr)
                continue;

            family = ifa->ifa_addr->sa_family;

            if (family == AF_INET && strcasecmp(ifa->ifa_name, "en0") == 0) {
                fprintf(stdout, "[Message][LocalNet]Device name: %s\n", ifa->ifa_name);

                addr = (struct sockaddr_in *) ifa->ifa_addr;
                _ipAddr = inet_ntop(AF_INET, &(addr->sin_addr), host, sizeof(host));
                fprintf(stdout, "[Message][LocalNet]	Addr: %s\n", _ipAddr.c_str());

                addr = (struct sockaddr_in *) ifa->ifa_broadaddr;
                _broadcastAddr = inet_ntop(AF_INET, &(addr->sin_addr), host, sizeof(host));
                fprintf(stdout, "[Message][LocalNet]	Broadcast address: %s\n", _broadcastAddr.c_str());

                freeifaddrs(ifaddr);
                return true;
            }
        }

        freeifaddrs(ifaddr);
        return false;
    }

    bool LocalNet::generateMac() {
#ifdef ANDROID
        int sockfd;
        struct ifreq ifr{};

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) >= 0) {
            //strncpy(ifr.ifr_name, "eth0", IFNAMESIZE);
            strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

            ifr.ifr_addr.sa_family = AF_INET;

            if (ioctl(sockfd, SIOCGIFHWADDR, (char*) &ifr) == 0) {
                char mac[128] = {};
                sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                        (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[0], (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[1],
                        (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[2], (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[3],
                        (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[4], (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[5]);
                _macAddr = mac;
                return true;
            }
        }
        return false;
#else
        int mib[6];
        size_t len;
        char *buf;
        unsigned char *ptr;
        struct if_msghdr *ifm;
        struct sockaddr_dl *sdl;

        mib[0] = CTL_NET;
        mib[1] = AF_ROUTE;
        mib[2] = 0;
        mib[3] = AF_LINK;
        mib[4] = NET_RT_IFLIST;

        if ((mib[5] = (int) if_nametoindex("en0")) == 0) {
            fprintf(stderr, "[Message][LocalNet]Error: if_nametoindex error/n");
            return false;
        }

        if (sysctl(mib, 6, nullptr, &len, nullptr, 0) < 0) {
            fprintf(stderr, "[Message][LocalNet]Error: sysctl, take 1/n");
            return false;
        }

        if ((buf = (char *) malloc(len)) == nullptr) {
            fprintf(stderr, "[Message][LocalNet]Could not allocate memory. error!/n");
            return false;
        }

        if (sysctl(mib, 6, buf, &len, nullptr, 0) < 0) {
            fprintf(stderr, "[Message][LocalNet]Error: sysctl, take 2");
            return false;
        }

        ifm = (struct if_msghdr *) buf;
        sdl = (struct sockaddr_dl *) (ifm + 1);
        ptr = (unsigned char *) LLADDR(sdl);
        char mac[128] = {};
        sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x\n", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4),
                *(ptr + 5));
        fprintf(stdout, "[Message][LocalNet]	Mac address: %s", mac);

        free(buf);

        _macAddr = mac;
        return true;
#endif
    }
}
