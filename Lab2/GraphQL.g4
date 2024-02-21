grammar GraphQL;

document // starting rule
    : queryDefinition+ EOF
    ;

queryDefinition
    : queryType name? selectionSet
    | selectionSet
    ;

queryType
    : selectQuery
    | removeQuery
    | updateQuery
    ;

selectQuery
    : 'select'
    ;

removeQuery
    : 'remove'
    ;

updateQuery
    : 'update'
    ;

selectionSet
    : '{' selection+ '}'
    ;

selection
    : name arguments? equal value
    | alias? name arguments? selectionSet?
    ;

arguments
    : '(' argument+ ')'
    ;

argument
    : name comparator value
    ;

comparator // not actual GraphQL, it allows only ':'
    : equal
    | notEqual
    | greater
    | less
    ;

equal
    : ':'
    | '='
    ;

notEqual
    : '!='
    ;

greater
    : '>'
    ;

less
    : '<'
    ;

alias
    : name ':'
    ;

value
    : intValue
    | doubleValue
    | stringValue
    | booleanValue
    | nullValue
//    | enumValue
//    | listValue
//    | objectValue
    ;

intValue
    : INT
    ;

doubleValue
    : FLOAT
    ;

booleanValue
    : 'true'
    | 'false'
    ;

stringValue
    : STRING
    | BLOCK_STRING
    ;

nullValue
    : 'null'
    ;

//enumValue
//    : name
//    ;
//
//listValue
//    : '[' ']'
//    | '[' value+ ']'
//    ;
//
//objectValue
//    : '{' objectField* '}'
//    ;
//
//objectField
//    : name ':' value
//    ;
//
//type_
//    : namedType '!'?
//    | listType '!'?
//    ;
//
//namedType
//    : name
//    ;
//
//listType
//    : '[' type_ ']'
//    ;

name
    : NAME
    ;

NAME
    : [_A-Za-z] [_0-9A-Za-z]*
    ;

fragment CHARACTER
    : (ESC | ~ ["\\])
    ;

STRING
    : '"' CHARACTER* '"'
    ;

BLOCK_STRING
    : '"""' .*? '"""'
    ;

ID
    : STRING
    ;

fragment ESC
    : '\\' (["\\/bfnrt] | UNICODE)
    ;

fragment UNICODE
    : 'u' HEX HEX HEX HEX
    ;

fragment HEX
    : [0-9a-fA-F]
    ;

fragment NONZERO_DIGIT
    : [1-9]
    ;

fragment DIGIT
    : [0-9]
    ;

fragment FRACTIONAL_PART
    : '.' DIGIT+
    ;

fragment EXPONENTIAL_PART
    : EXPONENT_INDICATOR SIGN? DIGIT+
    ;

fragment EXPONENT_INDICATOR
    : [eE]
    ;

fragment SIGN
    : [+-]
    ;

fragment NEGATIVE_SIGN
    : '-'
    ;

FLOAT
    : INT FRACTIONAL_PART
    | INT EXPONENTIAL_PART
    | INT FRACTIONAL_PART EXPONENTIAL_PART
    ;

INT
    : NEGATIVE_SIGN? '0'
    | NEGATIVE_SIGN? NONZERO_DIGIT DIGIT*
    ;

PUNCTUATOR
    : '!'
    | '$'
    | '('
    | ')'
    | '...'
    | ':'
    | '='
    | '@'
    | '['
    | ']'
    | '{'
    | '}'
    | '|'
    ;

fragment EXP
    : [Ee] [+\-]? INT
    ;

WS
    : [ \t\n\r]+ -> skip
    ;

COMMA
    : ',' -> skip
    ;

LineComment
    : '#' ~[\r\n]* -> skip
    ;

UNICODE_BOM
    : (UTF8_BOM | UTF16_BOM | UTF32_BOM) -> skip
    ;

UTF8_BOM
    : '\uEFBBBF'
    ;

UTF16_BOM
    : '\uFEFF'
    ;

UTF32_BOM
    : '\u0000FEFF'
    ;
