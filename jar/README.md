# 手动编译

在项目根目录下执行`java -jar jar/antlr-4.13.2-complete.jar -Dlanguage=Cpp third_party/gql_grammar/GQL.g4 -Xexact-output-dir -no-visitor -no-listener -o src/parser/generated`

文件夹[generated](../src/parser/generated)中的额外文件被 ignore 了，所以不用担心