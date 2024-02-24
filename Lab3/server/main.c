#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()


#include "backend.h"
#include "node.h"
#include "graph.h"
#include "utils.h"

#include "query.pb-c.h"

#define BUFFER_SIZE 1400
#define PORT 50001



struct runtime_node parse_selections(const struct Lab3__Query__Selection* const selection,
    Lab3__Query__TYPE type) {
  static int cnt = 0;
  struct runtime_node current_node = {0};
  current_node.properties = malloc(sizeof(struct runtime_property) * (selection->n_argument + selection->n_subselection));
  if (!current_node.properties)
    exit_with_error("memory problem");


  if (type != LAB3__QUERY__TYPE__INSERT) {
    for (uint64_t ind = 0; ind < selection->n_argument; ++ind) {
      const struct Lab3__Argument *const arg = selection->argument[ind];
      struct runtime_property property = {.key_string=arg->name, 0};

      switch (arg->value_case) {
        case LAB3__ARGUMENT__VALUE_STRING_VALUE:
          property.type = STRING;
          property.property_block.string_.length = strlen(arg->string_value) + 1;
          property.property_block.string_.data = arg->string_value;
//          strcpy(property.property_block.string_.data, arg->string_value);
          break;
        case LAB3__ARGUMENT__VALUE_INT_VALUE:
          property.type = INT;
          property.property_block.int_ = arg->int_value;
          break;
        case LAB3__ARGUMENT__VALUE_DOUBLE_VALUE:
          property.type = DOUBLE;
          property.property_block.double_ = arg->double_value;
          break;
        case LAB3__ARGUMENT__VALUE_BOOL_VALUE:
          property.type = INT;
          property.property_block.int_ = arg->bool_value;
          break;
        case LAB3__ARGUMENT__VALUE__NOT_SET:
        case _LAB3__ARGUMENT__VALUE__CASE_IS_INT_SIZE:
          exit_with_error("got corrupted data");
          break;
      }
      switch (arg->comparator) {
        case LAB3__COMPARATOR__EQUAL:
          property.comparator = EQUAL;
          break;
        case LAB3__COMPARATOR__GREATER:
          property.comparator = GREATER;
          break;
        case LAB3__COMPARATOR__LESS:
          property.comparator = LESS;
          break;
        case LAB3__COMPARATOR__NOT_EQUAL:
          property.comparator = NOT_EQUAL;
          break;
      }

      current_node.properties[current_node.properties_count++] = property;

    }
  }
  else {
    for (uint64_t ind = 0; ind < selection->n_subselection; ++ind) {
      const struct Lab3__Query__Selection *const sel = selection->subselection[ind];
      if (sel->opt_type_case == LAB3__QUERY__SELECTION__OPT_TYPE_TYPE) {
        struct runtime_property property = {.key_string=sel->name};

        switch (sel->value_case) {
          case LAB3__QUERY__SELECTION__VALUE_STRING_VALUE:
            property.type = STRING;
            property.property_block.string_.length = strlen(sel->string_value) + 1;
            property.property_block.string_.data = sel->string_value;
//            property.property_block.string_.data = malloc(sizeof(uint8_t) * property.property_block.string_.length);
//            strcpy(property.property_block.string_.data, sel->string_value);
            break;
          case LAB3__QUERY__SELECTION__VALUE_INT_VALUE:
            property.type = INT;
            property.property_block.int_ = sel->int_value;
            break;
          case LAB3__QUERY__SELECTION__VALUE_DOUBLE_VALUE:
            property.type = DOUBLE;
            property.property_block.double_ = sel->double_value;
            break;
          case LAB3__QUERY__SELECTION__VALUE_BOOL_VALUE:
            property.type = INT;
            property.property_block.int_ = sel->bool_value;
            break;
          case LAB3__QUERY__SELECTION__VALUE__NOT_SET:
          case _LAB3__QUERY__SELECTION__VALUE__CASE_IS_INT_SIZE:
            exit_with_error("got corrupted data");
            break;
        }
        
        current_node.properties[current_node.properties_count++] = property;
      }
    }
  }
  if (type == LAB3__QUERY__TYPE__INSERT)
    add_node(&current_node);
  if (type == LAB3__QUERY__TYPE__DELETE)
    remove_node(&current_node);

  int found_child = 0;

  for (uint64_t ind = 0; ind < selection->n_subselection; ++ind) {
    const struct Lab3__Query__Selection *const sel = selection->subselection[ind];
    if (sel->opt_type_case == LAB3__QUERY__SELECTION__OPT_TYPE__NOT_SET && sel->n_subselection != 0) {
      found_child = 1;
      ++cnt;
      struct runtime_node node = parse_selections(sel, type);
      --cnt;
      struct runtime_relationship relationship = {.direction=BIDIRECTIONAL,
          .first_node=current_node, .second_node=node,
          .relationship_type_string=sel->name};

      if (type == LAB3__QUERY__TYPE__INSERT)
        add_relationship(&relationship);
      if (type == LAB3__QUERY__TYPE__DELETE)
        remove_relationship(&relationship);
      if (type == LAB3__QUERY__TYPE__SELECT) {
//        struct runtime_node tmp = relationship.second_node;
//        relationship.second_node = relationship.first_node;
//        relationship.first_node = tmp; // fucking swap hack to select properly. don't have time to fix it now

//        int64_t cna = 0;
//        int64_t current_rel_addr = find_relationship(&relationship, &cna);
//        while (current_rel_addr >= 0) {
//          struct relationship rl;
//          relationship_read(&rl, current_rel_addr);
//          print_node(rl.first_node_addr);
//          print_relationship(current_rel_addr);
//          print_node(rl.second_node_addr);
//          current_rel_addr = find_relationship(&relationship, &cna);
//        }
        print_relationships(&relationship);
      }
      free(node.properties);
    }
  }

  return current_node;
}

void parse_query(struct Lab3__Query* query) {
  for (uint64_t ind = 0; ind < query->n_selection; ++ind) {
    struct runtime_node tmp = parse_selections(query->selection[ind], query->type);
    free(tmp.properties);
  }
}

void update_query(struct Lab3__Query* query) {
  assert(query->type == LAB3__QUERY__TYPE__MODIFY);

  query->type = LAB3__QUERY__TYPE__DELETE;
  parse_query(query);

  query->type = LAB3__QUERY__TYPE__INSERT;
  parse_query(query);
}


int main(int argc, char** argv)
{
  char err_msg[100] = {0};
  if (argc != 2)
    exit_with_error("Pass filename as an argument!");
  backend_start(argv[1]);

  int sock_fd, conn_fd;
  uint len;
  struct sockaddr_in servaddr, cli;

  // socket create and verification
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    strcpy(err_msg, "socket creation failed...\n");
    goto EXIT;
  }
  else
    printf("Socket created..\n");
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  // Binding newly created socket to given IP and verification
  if ((bind(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
    strcpy(err_msg, "socket bind failed...\n");
    goto EXIT;
  }
  else
    printf("Socket bound..\n");

  // Now server is ready to listen and verification
  if ((listen(sock_fd, 5)) != 0) {
    strcpy(err_msg, "Listen failed...\n");
    goto EXIT;
  }
  else
    printf("Server listening..\n");
  len = sizeof(cli);

  // Accept the data packet from client and verification
  conn_fd = accept(sock_fd, (struct sockaddr*)&cli, &len);
  if (conn_fd < 0) {
    strcpy(err_msg, "server accept failed...\n");
    goto EXIT;
  }
  else
    printf("server accepted the client.\n");

  // Function for chatting between client and server
  uint8_t buff[BUFFER_SIZE];
  int64_t msg_size;
  // infinite loop for chat
  for (;;) {

    // read the message from client and copy it in buffer

    if ((msg_size = read(conn_fd, buff, sizeof(buff))) == 0) {
      printf("Client exited\n");

      conn_fd = accept(sock_fd, (struct sockaddr*)&cli, &len);
      if (conn_fd < 0) {
        strcpy(err_msg, "server accept failed...\n");
        goto EXIT;
      }
      else
        printf("server accepted the client\n");
      continue;
    }

    struct Lab3__Query* query;
    query = lab3__query__unpack(NULL, msg_size, buff);

    switch (query->type) {
      case LAB3__QUERY__TYPE__SELECT:
        string_buffer_init();
      case LAB3__QUERY__TYPE__INSERT:
      case LAB3__QUERY__TYPE__DELETE:
        parse_query(query);
        break;
      case LAB3__QUERY__TYPE__MODIFY:
        update_query(query);
        break;
    }

    fflush(string_buffer_get().file);
    if (query->type != LAB3__QUERY__TYPE__SELECT) {
      string_buffer_init();
      string_buffer_printf("%s", "done");
//      print_graph(0); // debug
    }
    else if (string_buffer_get().size == 0) {
      string_buffer_printf("%s", "nothing");
    }
//    print_graph(0);
    fflush(string_buffer_get().file);

    write(conn_fd, string_buffer_get().data, (string_buffer_get().size < BUFFER_SIZE ?
                                              string_buffer_get().size : BUFFER_SIZE));
    lab3__query__free_unpacked(query, NULL);
  }

EXIT:
  close(sock_fd);
  backend_stop();
  if (err_msg[0] == '\0')
    return 0;
  else
    exit_with_error(err_msg);
}
