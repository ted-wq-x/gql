# 手动编译

`java -jar jar/antlr-4.13.2-complete.jar -Dlanguage=Cpp third_party/gql_grammar/GQL.g4 -no-visitor -no-listener -o generated`

编译完后的文件手动复制到[generated](../src/parser/generated)