#include <ksvcmon.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>

#define BUFLEN		4096

double do_metric_rtnetlink(struct metric *m)
{
    struct timespec start, end;
    int ret;
    int fd;
    struct iovec iov;
    struct msghdr msg;
    char buf[BUFLEN] = {0};
    (void)m;

    clock_gettime(CLOCK_MONOTONIC, &start);

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    
    if (fd < 0) {
        return -1;
    }

	struct sockaddr_nl sa = {0};
	sa.nl_family = AF_NETLINK;

	// assemble the message according to the netlink protocol
	struct nlmsghdr *nl;
	nl = (struct nlmsghdr*)buf;
	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	nl->nlmsg_type = RTM_GETADDR;
	nl->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;

	struct ifaddrmsg *ifa;
	ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
	ifa->ifa_family = AF_INET; // we only get ipv4 address here

	// prepare struct msghdr for sending.
    iov.iov_base = buf;
    iov.iov_len = nl->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

	// send netlink message to kernel.
	ret = sendmsg(fd, &msg, 0);
	if (ret < 0) {
        close(fd);
        return -1;
    }

	iov.iov_base = buf;
	iov.iov_len = BUFLEN;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &sa;
	msg.msg_namelen = sizeof(sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

    ret = recvmsg(fd, &msg, 0);
    if (ret < 0) {
        close(fd);
        return -1;
    }
    if (!NLMSG_OK(nl, ret)) {
        close(fd);
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    close(fd);

    if (!m->debug)
        goto return_time;

    for (nl = (struct nlmsghdr*)buf; NLMSG_OK(nl, ret); nl = NLMSG_NEXT(nl, ret)) {
        struct ifaddrmsg *ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
        struct rtattr *rta = IFA_RTA(ifa);
        int rtl = IFA_PAYLOAD(nl);

        for (; RTA_OK(rta, rtl); rta = RTA_NEXT(rta, rtl)) {
            if (rta->rta_type == IFA_LOCAL) {
                struct in_addr addr;
                memcpy(&addr, RTA_DATA(rta), sizeof(struct in_addr));
                printf("Interface %d: %s\n", ifa->ifa_index, inet_ntoa(addr));
            }
        }
    }

return_time:
    return time_diff_us(&start, &end);
}

struct metric metric_rtnetlink = {
    .name = "rtnetlink",
    .desc = "get interface addresses over netlink",
    .do_once = do_metric_rtnetlink,
    .sleep_us = US_1_SEC,
    // .debug = 1,
};

REGISTER_METRIC(metric_rtnetlink)