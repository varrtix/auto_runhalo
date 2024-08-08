#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#define XML_NODE_FOREACH(root, node)                                           \
  for (xmlNodePtr node = root->children; node; node = node->next)

#define XML_NODE_FILTER(root, node, node_name)                                 \
  XML_NODE_FOREACH(root, node)                                                 \
  if (node->type == XML_ELEMENT_NODE &&                                        \
      strcmp((const char *)node->name, node_name) == 0)

static const char *DEFAULT_IFNAME = "etheqos";
static const char *DEFAULT_ENCODING_FORMAT = "UTF-8";

static const char *XML_K_ELEM_NODES = "NODES";
static const char *XML_K_ELEM_MODULES = "MODULES";
static const char *XML_K_NODES_NODE = "NODE";
static const char *XML_K_MODULES_MODULE = "MODULE";
static const char *XML_K_NODE_IP = "ip";
static const char *XML_K_NODE_ID = "id";
static const char *XML_K_MODULE_NODE = "node";
static const char *XML_K_MODULE_NAME = "name";

static inline int safe_atoi(const char *str, int *out) {
  char *endptr;
  errno = 0;
  long res = strtol(str, &endptr, 10);
  if (errno == ERANGE || res == LONG_MAX || res == LONG_MIN || endptr == str ||
      (endptr && *endptr != '\0'))
    return 0;
  *out = (int)res;
  return 1;
}

static inline int valid_mod_name(const char *mod_name, const char **ban_names,
                                 size_t ban_n) {
  if (mod_name && ban_n && ban_names)
    for (size_t i = 0; i < ban_n; ++i)
      if (*(ban_names + i) && strcmp(mod_name, *(ban_names + i)) == 0)
        return 0;
  return 1;
}

static char **parse_ban_list(const char *str, size_t *n) {
  char *dup_str = strdup(str), **list = NULL, *token = NULL, *sp;
  size_t bn = 0;
  if (!n || !dup_str) {
    perror("failed to dup ban list");
    goto end;
  }

  token = strtok_r(dup_str, ",", &sp);
  while (token) {
    bn++;
    token = strtok_r(NULL, ",", &sp);
  }

  if (!bn)
    goto end;

  list = (char **)malloc(bn * sizeof(char *));
  if (!list) {
    perror("failed to alloc ban list memory");
    goto end;
  }

  token = dup_str;
  for (size_t i = 0; i < bn; ++i) {
    *(list + i) = strdup(token);
    if (!*(list + i)) {
      perror("failed to dup ban name");
      for (size_t j = 0; j < i; ++j)
        free(*(list + j));
      free(list);
      list = NULL;
      goto end;
    }
    token += strlen(token) + 1;
  }

  *n = bn;

end:
  if (dup_str)
    free(dup_str);

  return list;
}

static char **match_mod_names(const char *xml_path, const char *encoding,
                              const char *ip, const char **ban_names,
                              size_t ban_n, size_t *n) {
  size_t tmp_n = 0;
  int node_id = -1, tmp_id = -1;
  char **mod_names = NULL;
  xmlChar *tmp_prop;
  xmlNodePtr root, nodes;
  xmlDocPtr doc =
      xmlReadFile(xml_path, encoding, XML_PARSE_RECOVER | XML_PARSE_NOWARNING);
  if (!n || !doc || !(root = xmlDocGetRootElement(doc))) {
    fprintf(stderr, "failed to parse XML file: %s\n", xml_path);
    goto end;
  }

  XML_NODE_FILTER(root, node, XML_K_ELEM_NODES) {
    if (!(nodes = node)) {
      fprintf(stderr, "no %s element found\n", XML_K_ELEM_NODES);
      goto end;
    }
    break;
  }

  XML_NODE_FILTER(nodes, node, XML_K_NODES_NODE) {
    tmp_prop = xmlGetProp(node, XML_K_NODE_IP);
    if (!tmp_prop)
      continue;

    if (strcmp((const char *)tmp_prop, ip) == 0) {
      xmlFree(tmp_prop);
      tmp_prop = xmlGetProp(node, XML_K_NODE_ID);
      if (tmp_prop) {
        if (safe_atoi((const char *)tmp_prop, &node_id) == 0)
          node_id = -1;
        xmlFree(tmp_prop);
        break;
      }
    }
    xmlFree(tmp_prop);
  }

  if (node_id == -1) {
    fprintf(stderr, "no matching NODE found for IP address: %ss\n", ip);
    goto end;
  }

  XML_NODE_FILTER(root, node, XML_K_ELEM_MODULES) {
    if (!(nodes = node)) {
      fprintf(stderr, "no %s element found\n", XML_K_ELEM_MODULES);
      goto end;
    }
    break;
  }

  XML_NODE_FILTER(nodes, node, XML_K_MODULES_MODULE) {
    tmp_prop = xmlGetProp(node, XML_K_MODULE_NODE);
    if (!tmp_prop)
      continue;

    if (safe_atoi((const char *)tmp_prop, &tmp_id) == 0 || tmp_id != node_id) {
      xmlFree(tmp_prop);
      tmp_id = -1;
      continue;
    }

    xmlFree(tmp_prop);
    tmp_prop = xmlGetProp(node, XML_K_MODULE_NAME);
    if (tmp_prop) {
      if (valid_mod_name(tmp_prop, ban_names, ban_n))
        tmp_n++;
      xmlFree(tmp_prop);
    }
  }

  if (!tmp_n || !(mod_names = (char **)malloc(tmp_n * sizeof(char *))))
    goto end;

  *n = tmp_n;
  tmp_n = 0;
  XML_NODE_FILTER(nodes, node, XML_K_MODULES_MODULE) {
    tmp_prop = xmlGetProp(node, XML_K_MODULE_NODE);
    if (!tmp_prop)
      continue;

    if (safe_atoi((const char *)tmp_prop, &tmp_id) == 0 || tmp_id != node_id) {
      xmlFree(tmp_prop);
      tmp_id = -1;
      continue;
    }

    xmlFree(tmp_prop);
    tmp_prop = xmlGetProp(node, XML_K_MODULE_NAME);
    if (tmp_prop) {
      if (valid_mod_name(tmp_prop, ban_names, ban_n))
        *(mod_names + tmp_n++) = strdup((const char *)tmp_prop);
      xmlFree(tmp_prop);
    }
  }

  if (!tmp_n) {
    free(mod_names);
    mod_names = NULL;
  }

end:
  if (doc)
    xmlFreeDoc(doc);

  return mod_names;
}

static char *get_ip_from_interface(const char *if_name) {
  int family;
  char *ip = NULL;
  struct ifaddrs *ifaddr, *ifa;

  if (getifaddrs(&ifaddr) == -1) {
    perror("failed to get network interface");
    goto end;
  }

  for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr)
      continue;

    family = ifa->ifa_addr->sa_family;
    if (family == AF_INET && strcmp(ifa->ifa_name, if_name) == 0) {
      ip = (char *)malloc(INET_ADDRSTRLEN);
      if (!ip) {
        perror("failed to alloc memory");
        goto cleanup;
      }

      if (!inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                     ip, INET_ADDRSTRLEN)) {
        perror("failed to convert IP address");
        free(ip);
        ip = NULL;
        goto cleanup;
      }

      break; // Only take the first IP address found
    }
  }

cleanup:
  freeifaddrs(ifaddr);

end:
  return ip;
}

static void print_usage(const char *prog_name) {
  printf("Usage: %s -i FILEPATH [-b MODNAME...] [-n IFNAME] [-e ENCODING]\n",
         prog_name);
  printf("Options:\n");
  printf("  -i  FILEPATH to the XML configuration file\n");
  printf("  -e  ENCODING format (default: UTF-8)\n");
  printf("  -b  Comma-separated list of banned MODNAME\n");
  printf("  -n  Network IFNAME to get the IP addr (default: etheqos)\n");
  printf("  -h  Show this help message\n");
}

int main(int argc, char *const *argv) {
  int opt, ret = EXIT_FAILURE;
  const char *xml_path = NULL, *if_name = NULL, *encoding, *ban_list_str = NULL;
  char *local_ip = NULL, **ban_names = NULL;
  char **mod_names = NULL;
  size_t n = 0, ban_n = 0;

  if (argc == 1)
    goto usage;

  while ((opt = getopt(argc, argv, "hi:n:b:e:")) != -1) {
    switch (opt) {
    case 'i':
      if (!(xml_path = optarg))
        goto usage;
      break;
    case 'b':
      if (!(ban_list_str = optarg))
        goto usage;
      break;
    case 'n':
      if (!(if_name = optarg))
        goto usage;
      break;
    case 'e':
      if (!(encoding = optarg))
        goto usage;
      break;
    case 'h':
      ret = EXIT_SUCCESS;
    default:
    usage:
      print_usage(argv[0]);
      goto end;
    }
  }

  if (!if_name)
    if_name = DEFAULT_IFNAME;

  if (!encoding)
    encoding = DEFAULT_ENCODING_FORMAT;

  local_ip = get_ip_from_interface(if_name);
  if (!local_ip) {
    fprintf(stderr, "failed to get IP address from interface: %s\n", if_name);
    goto end;
  }

  if (ban_list_str)
    ban_names = parse_ban_list(ban_list_str, &ban_n);

  mod_names = match_mod_names(xml_path, encoding, local_ip,
                              (const char **)ban_names, ban_n, &n);
  if (ban_n && ban_names) {
    for (size_t i = 0; i < ban_n; ++i)
      free(*(ban_names + i));
    free(ban_names);
  }

end:
  if (n && mod_names) {
    ret = EXIT_SUCCESS;
    for (size_t i = 0; i < n; ++i) {
      printf("%s\n", *(mod_names + i));
      free(*(mod_names + i));
    }
    free(mod_names);
  }

  if (local_ip)
    free(local_ip);

  return ret;
}
