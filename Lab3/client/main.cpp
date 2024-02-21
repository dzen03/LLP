#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "GraphQLLexer.h"
#include "GraphQLParser.h"
#include "GraphQLBaseListener.h"

#include "query.pb.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

static constexpr const char* SERVER = "127.0.0.1";
static constexpr int BUFFER_SIZE = 1024;
static constexpr int PORT = 50001;

lab3::Query* query;
std::stack<lab3::Query_Selection*> qs_stack;
lab3::Argument* current_argument = nullptr;

class MyListener : public graphql::GraphQLBaseListener {
 public:
//  void visitTerminal(antlr4::tree::TerminalNode* node) override {
//    std::cout << node->getText() << std::endl;
//  }

  void enterSelection(graphql::GraphQLParser::SelectionContext * ctx) override {
    if (qs_stack.top() == nullptr)
      qs_stack.push(query->add_selection());
    else
      qs_stack.push(qs_stack.top()->add_subselection());
  }

  void exitSelection(graphql::GraphQLParser::SelectionContext * ctx) override {
    qs_stack.pop();
  }

  void enterArguments(graphql::GraphQLParser::ArgumentsContext * /*ctx*/) override {
    current_argument = qs_stack.top()->add_argument();
  }

  void enterEqual(graphql::GraphQLParser::EqualContext * /*ctx*/) override {
    if (current_argument != nullptr) {
      current_argument->set_comparator(lab3::COMPARATOR::EQUAL);
    }
  }

  void enterNotEqual(graphql::GraphQLParser::NotEqualContext * /*ctx*/) override {
    current_argument->set_comparator(lab3::COMPARATOR::NOT_EQUAL);
  }

  void enterGreater(graphql::GraphQLParser::GreaterContext * /*ctx*/) override {
    current_argument->set_comparator(lab3::COMPARATOR::GREATER);
  }

  void enterLess(graphql::GraphQLParser::LessContext * /*ctx*/) override {
    current_argument->set_comparator(lab3::COMPARATOR::LESS);
  }

  void enterIntValue(graphql::GraphQLParser::IntValueContext * ctx) override {
    if (current_argument != nullptr) {
      current_argument->set_int_value(std::stoll(ctx->getText()));
      current_argument->set_type(lab3::VALUE_TYPE::INT);
    }
    else {
      qs_stack.top()->set_int_value(std::stoll(ctx->getText()));
      qs_stack.top()->set_type(lab3::VALUE_TYPE::INT);
    }
  }

  void enterDoubleValue(graphql::GraphQLParser::DoubleValueContext * ctx) override {
    if (current_argument != nullptr) {
      current_argument->set_double_value(std::stod(ctx->getText()));
      current_argument->set_type(lab3::VALUE_TYPE::DOUBLE);
    }
    else {
      qs_stack.top()->set_double_value(std::stod(ctx->getText()));
      qs_stack.top()->set_type(lab3::VALUE_TYPE::DOUBLE);
    }
  }

  void enterBooleanValue(graphql::GraphQLParser::BooleanValueContext * ctx) override {
    if (current_argument != nullptr) {
      current_argument->set_bool_value(stob(ctx->getText()));
      current_argument->set_type(lab3::VALUE_TYPE::BOOL);
    }
    else {
      qs_stack.top()->set_bool_value(stob(ctx->getText()));
      qs_stack.top()->set_type(lab3::VALUE_TYPE::BOOL);
    }
  }

  void enterStringValue(graphql::GraphQLParser::StringValueContext * ctx) override {
    if (current_argument != nullptr) {
      current_argument->set_string_value(ctx->getText());
      current_argument->set_type(lab3::VALUE_TYPE::STRING);
    }
    else {
      qs_stack.top()->set_string_value(ctx->getText());
      qs_stack.top()->set_type(lab3::VALUE_TYPE::STRING);
    }
  }

  void enterNullValue(graphql::GraphQLParser::NullValueContext * ctx) override {
    if (current_argument != nullptr) {
      current_argument->set_type(lab3::VALUE_TYPE::NONE);
    }
    else {
      qs_stack.top()->set_type(lab3::VALUE_TYPE::NONE);
    }
  }



  void exitArguments(graphql::GraphQLParser::ArgumentsContext * /*ctx*/) override {
    current_argument = nullptr;
  }

  void enterName(graphql::GraphQLParser::NameContext* ctx) override {
    if (current_argument != nullptr) {
      current_argument->set_name(ctx->getText());
    }
    else if (qs_stack.top() != nullptr) {
      qs_stack.top()->set_name(ctx->getText());
    }
  }

 private:
  static inline bool stob(std::string inp) {
    assert(std::equal(inp.begin(), inp.end(), "0") || std::equal(inp.begin(), inp.end(), "false")
          || std::equal(inp.begin(), inp.end(), "1") || std::equal(inp.begin(), inp.end(), "true"));
    return std::equal(inp.begin(), inp.end(), "1") || std::equal(inp.begin(), inp.end(), "true");
  }

};

int send_to_server(std::string& );


int main(int , const char **) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  int ret_code = 0;

  query = new lab3::Query();
  qs_stack.push(nullptr);

  std::ifstream inputStream("test_query");

  antlr4::ANTLRInputStream input(inputStream);
  graphql::GraphQLLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  graphql::GraphQLParser parser(&tokens);
  MyListener listener;
  antlr4::tree::ParseTree* tree = parser.document();
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

  std::string out;

  if (!query->SerializeToString(&out)) {
    std::cerr << "Failed to serialize." << std::endl;
    ret_code = -1;
    goto EXIT;
  }

  std::cout << query->DebugString() << std::endl;

  if ((ret_code = send_to_server(out)) != 0) {
    goto EXIT;
  }

EXIT:

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  delete query;

  return ret_code;
}

int send_to_server(std::string& buff) {
  int sock;
  int8_t server_reply[BUFFER_SIZE] = {0};


  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    std::cout << "Could not create socket" << std::endl;
  }
  std::cout << "Socket created" << std::endl;


  struct sockaddr_in server = {.sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr = {inet_addr(SERVER)}};

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("Connect failed. Error");
    return 1;
  }
  std::cout << "Connected to server\n" << std::endl;

  if (send(sock, buff.c_str(), buff.size(), 0) < 0) {
    std::cout << "Send failed" << std::endl;
    return 1;
  }
  std::cout << "Data Sent\n" << std::endl;

  if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
    std::cout << "recv failed" << std::endl;
  }

  std::cout << "Server reply: ";
  std::cout << server_reply << std::endl;

  close(sock);
  return 0;
}
