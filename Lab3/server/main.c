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

void dump_node(int64_t node_addr, struct Lab3__Result__Chain__Node* result_node)
{

  struct node node;
  node_read(&node, node_addr);

  int64_t next_property_addr = node.next_property_addr;

  struct property property;
  struct dynamic_store key;
  struct dynamic_store value;

  result_node->fields = malloc(sizeof(struct Lab3__Result__Chain__Node__Field*) * 100);
  result_node->n_fields = 0;

  while (next_property_addr != 0)
  {
    property_read(&property, next_property_addr);

    dynamic_store_read(&key, property.key_block_addr);

    result_node->fields[result_node->n_fields] = malloc(sizeof(struct Lab3__Result__Chain__Node__Field));
    lab3__result__chain__node__field__init(result_node->fields[result_node->n_fields]);

    result_node->fields[result_node->n_fields]->key = malloc(sizeof (char) * key.header.length);
    memcpy(result_node->fields[result_node->n_fields]->key, &key.data, key.header.length);

//    string_buffer_printf("{%s: ", key.data);


    switch (property.type)
    {
      case INT:
        result_node->fields[result_node->n_fields]->type = LAB3__VALUE__TYPE__INT;
        result_node->fields[result_node->n_fields]->value_case = LAB3__RESULT__CHAIN__NODE__FIELD__VALUE_INT_VALUE;
        result_node->fields[result_node->n_fields]->int_value = property.property_block.int_;
//      string_buffer_printf("%lld}, ", (unsigned long long)property.property_block.int_);
        break;
      case DOUBLE:
        result_node->fields[result_node->n_fields]->type = LAB3__VALUE__TYPE__DOUBLE;
        result_node->fields[result_node->n_fields]->value_case = LAB3__RESULT__CHAIN__NODE__FIELD__VALUE_DOUBLE_VALUE;
        result_node->fields[result_node->n_fields]->double_value = property.property_block.double_;
//      string_buffer_printf("%f}, ", property.property_block.double_);
        break;
      case STRING:
        dynamic_store_read(&value, property.property_block.addr);
        result_node->fields[result_node->n_fields]->type = LAB3__VALUE__TYPE__STRING;
        result_node->fields[result_node->n_fields]->value_case = LAB3__RESULT__CHAIN__NODE__FIELD__VALUE_STRING_VALUE;
        result_node->fields[result_node->n_fields]->string_value = malloc(sizeof (char) * value.header.length);
        memcpy(result_node->fields[result_node->n_fields]->string_value, &value.data, value.header.length);
//        string_buffer_printf("%s}, ", value.data);
        break;
      default:
        exit_with_error("Unknown type in the property value field.");
    }

    ++result_node->n_fields;

    next_property_addr = property.next_property_addr;
  }
}

void dump_relationship(int64_t relationship_addr, struct Lab3__Result__Chain__Relation* result_relation)
{
  struct relationship relationship;
  relationship_read(&relationship, relationship_addr);

  struct dynamic_store key;

  dynamic_store_read(&key, relationship.relationship_type_block_addr);

//  string_buffer_printf("-[%s]-", key.data);
  switch (relationship.direction) {
    case BIDIRECTIONAL:
      result_relation->direction=LAB3__RESULT__CHAIN__RELATION__DIRECTIONS__BIDIRECTIONAL;
      break;
    case LEFT_TO_RIGHT:
      result_relation->direction=LAB3__RESULT__CHAIN__RELATION__DIRECTIONS__LEFT_TO_RIGHT;
      break;
    case RIGHT_TO_LEFT:
      result_relation->direction=LAB3__RESULT__CHAIN__RELATION__DIRECTIONS__RIGHT_TO_LEFT;
      break;
  }
  result_relation->name = malloc(sizeof (char) * key.header.length);
  memcpy(result_relation->name, key.data, key.header.length);
}

void dump_relationships(const struct runtime_relationship* const relationship, struct Lab3__Result* result)
{
  struct file file = get_file();

  int64_t current_node_addr = file.metadata.first_node_addr;

  result->chains = malloc(sizeof(struct Lab3__Result__Chain*) * 100);
  result->n_chains = 0;

  int64_t current_relationship_addr;
  while (current_node_addr > 0)
  {
    struct node node;
    node_read(&node, current_node_addr);

    if (!nodes_equal(current_node_addr, &relationship->first_node))
      continue;

    current_relationship_addr = node.next_relationship_addr;

    while (current_relationship_addr != 0) // searching if arguments are subset of our properties
    {
      struct relationship rl;
      relationship_read(&rl, current_relationship_addr);

      if (relationship_equal(current_relationship_addr, relationship)
          && current_node_addr == rl.first_node_addr) {
        result->chains[result->n_chains] = malloc(sizeof(struct Lab3__Result__Chain));
        lab3__result__chain__init(result->chains[result->n_chains]);

        result->chains[result->n_chains]->nodes = malloc(sizeof(struct Lab3__Result__Chain__Node*) * 2);
        result->chains[result->n_chains]->relations = malloc(sizeof(struct Lab3__Result__Chain__Relation*) * 1);

        result->chains[result->n_chains]->nodes[0] = malloc(sizeof(Lab3__Result__Chain__Node));
        result->chains[result->n_chains]->nodes[1] = malloc(sizeof(Lab3__Result__Chain__Node));
        result->chains[result->n_chains]->relations[0] = malloc(sizeof(Lab3__Result__Chain__Relation));

        lab3__result__chain__node__init(result->chains[result->n_chains]->nodes[0]);
        lab3__result__chain__node__init(result->chains[result->n_chains]->nodes[1]);
        lab3__result__chain__relation__init(result->chains[result->n_chains]->relations[0]);

        dump_node(rl.first_node_addr, result->chains[result->n_chains]->nodes[0]);
        dump_relationship(current_relationship_addr, result->chains[result->n_chains]->relations[0]);
        dump_node(rl.second_node_addr, result->chains[result->n_chains]->nodes[1]);

        result->chains[result->n_chains]->n_nodes = 2;
        result->chains[result->n_chains]->n_relations = 1;
        ++result->n_chains;
      }

      current_relationship_addr = rl.first_next_relationship_addr;
    }

    if (current_node_addr == node.next_node_addr)
      break;
    current_node_addr = node.next_node_addr;
  }
}


struct runtime_node parse_selections(const struct Lab3__Query__Selection* const selection,
    Lab3__Query__TYPE type, struct Lab3__Result* result) {
  static int cnt = 0;
  struct runtime_node current_node = {0};
  current_node.properties = malloc(sizeof(struct runtime_property) * (selection->n_argument + selection->n_subselection));
  if (!current_node.properties){
    exit_with_error("memory problem");
    return (struct runtime_node){0};
  }


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

  for (uint64_t ind = 0; ind < selection->n_subselection; ++ind) {
    const struct Lab3__Query__Selection *const sel = selection->subselection[ind];
    if (sel->opt_type_case == LAB3__QUERY__SELECTION__OPT_TYPE__NOT_SET && sel->n_subselection != 0) {
      ++cnt;
      struct runtime_node node = parse_selections(sel, type, result);
      --cnt;
      struct runtime_relationship relationship = {.direction=BIDIRECTIONAL,
          .first_node=current_node, .second_node=node,
          .relationship_type_string=sel->name};

      if (type == LAB3__QUERY__TYPE__INSERT)
        add_relationship(&relationship);
      if (type == LAB3__QUERY__TYPE__DELETE)
        remove_relationship(&relationship);
      if (type == LAB3__QUERY__TYPE__SELECT) {
        dump_relationships(&relationship, result);
      }
      free(node.properties);
    }
  }

  return current_node;
}

void parse_query(struct Lab3__Query* query, struct Lab3__Result* result) {
  for (uint64_t ind = 0; ind < query->n_selection; ++ind) {
    struct runtime_node tmp = parse_selections(query->selection[ind], query->type, result);
    free(tmp.properties);
  }
}

void update_query(struct Lab3__Query* query, struct Lab3__Result* result) {
  assert(query->type == LAB3__QUERY__TYPE__MODIFY);

  query->type = LAB3__QUERY__TYPE__DELETE;
  parse_query(query, result);

  query->type = LAB3__QUERY__TYPE__INSERT;
  parse_query(query, result);
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

    struct Lab3__Result result = LAB3__RESULT__INIT;

    switch (query->type) {
      case LAB3__QUERY__TYPE__SELECT:
        string_buffer_init();
      case LAB3__QUERY__TYPE__INSERT:
      case LAB3__QUERY__TYPE__DELETE:
        parse_query(query, &result);
        break;
      case LAB3__QUERY__TYPE__MODIFY:
        update_query(query, &result);
        break;
    }

    fflush(string_buffer_get().file);
//    if (query->type != LAB3__QUERY__TYPE__SELECT) {
//      string_buffer_init();
////      string_buffer_printf("%s", "done");
////      print_graph(0); // debug
//    }
//    else if (string_buffer_get().size == 0) {
////      string_buffer_printf("%s", "nothing");
//    }
//    print_graph(0);
    fflush(string_buffer_get().file);

    uint8_t* buffer = malloc(sizeof(uint8_t) * BUFFER_SIZE);
    uint64_t tmp = lab3__result__get_packed_size(&result);

    lab3__result__pack(&result, buffer);

    write(conn_fd, buffer, tmp);
//    lab3__query__free_unpacked(query, NULL);
  }

EXIT:
  close(sock_fd);
  backend_stop();
  if (err_msg[0] == '\0')
    return 0;
  else
    exit_with_error(err_msg);
}
