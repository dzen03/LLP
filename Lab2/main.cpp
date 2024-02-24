#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "GraphQLLexer.h"
#include "GraphQLParser.h"

using namespace graphql;
using namespace antlr4;

int main(int , const char **) {
  std::ifstream inputStream("../test_query");

  ANTLRInputStream input(inputStream);
  GraphQLLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  tokens.fill();
  for (auto token : tokens.getTokens()) {
    std::cout << token->toString() << std::endl;
  }

  GraphQLParser parser(&tokens);
  tree::ParseTree* tree = parser.document();

  std::cout << tree->toStringTree(&parser, true) << std::endl;

  return 0;
}