#include <gtest/gtest.h>
#include <iostream>

#include "compiler/backend/ast/semantizer/semantizer.hpp"
#include "compiler/codegen/ast_to_llvmir/ir_generator.hpp"
#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/parser/parser.hpp"
#include "compiler/utils/source_files.hpp"

using namespace ast;
using namespace ir_generator;

using lexer::Lexer;
using parser::Parser;
using semantizer::Semantizer;
using utils::SourceFile;

TEST(IRGenerator, can_be_constructed) {
    ASSERT_NO_THROW(IRGenerator generator("module"));
}

TEST(IRGenerator, can_run_generation) {
    SourceFile source = {
        "def main() -> None:",
        "    z: int = 1",
        "    print(z)",
    };
    auto tokens = Lexer::process(source);
    auto tree = Parser::process(tokens);
    Semantizer::process(tree);

    IRGenerator generator("module");
    ASSERT_NO_THROW(generator.process(tree));
}

TEST(IRGenerator, can_generate_and_dump_ir) {
    SourceFile source = {
        "def foo(x: int) -> float:",
        "    z: int = 5 + x",
        "    return 3.5 + z",
        "def main() -> None:",
        "    x: int = 1",
        "    y: float",
        "    y = input()",
        "    print(\"hi\")",
        "    if y > 1.5:",
        "        z: float = foo(x) * 2.5",
        "    else:",
        "        y = 6",
        "        while x < 10:",
        "            x = x + 1",
        "    print(y)",
    };
    auto tokens = Lexer::process(source);
    auto tree = Parser::process(tokens);
    Semantizer::process(tree);

    IRGenerator generator("module");
    generator.process(tree);

    std::string ir = R"(
@.str.0 = private unnamed_addr constant [3 x i8] c"hi\00", align 1

declare i32 @printf(ptr, ...)

declare i32 @scanf(ptr, ...)

define double @foo(i64 %0) {
foo_entry:
  %x = alloca i64, align 8
  store i64 %0, ptr %x, align 4
  br label %block

block:                                            ; preds = %foo_entry
  %1 = load i64, ptr %x, align 4
  %2 = add i64 5, %1
  %z = alloca i64, align 8
  store i64 %2, ptr %z, align 4
  %3 = load i64, ptr %z, align 4
  %typeconv = sitofp i64 %3 to double
  %4 = fadd double 3.500000e+00, %typeconv
  ret double %4
}

define void @main() {
main_entry:
  br label %block

block:                                            ; preds = %main_entry
  %x = alloca i64, align 8
  store i64 1, ptr %x, align 4
  %y = alloca double, align 8
  %input = alloca double, align 8
  %0 = call i32 (ptr, ...) @scanf(ptr @.placeholder.float, ptr %input)
  %1 = load double, ptr %input, align 8
  store double %1, ptr %y, align 8
  %2 = call i32 (ptr, ...) @printf(ptr @.placeholder.str, ptr @.str.0)
  %3 = call i32 (ptr, ...) @printf(ptr @.placeholder.newline)
  br label %ifcond

ifcond:                                           ; preds = %block
  %4 = load double, ptr %y, align 8
  %5 = fcmp ogt double %4, 1.500000e+00
  br i1 %5, label %ifbody, label %elsebody

ifbody:                                           ; preds = %ifcond
  %6 = load i64, ptr %x, align 4
  %7 = call double @foo(i64 %6)
  %8 = fmul double %7, 2.500000e+00
  %z = alloca double, align 8
  store double %8, ptr %z, align 8
  br label %endif

elsebody:                                         ; preds = %ifcond
  %typeconv = sitofp i64 6 to double
  store double %typeconv, ptr %y, align 8
  br label %whilecond

whilecond:                                        ; preds = %whilebody, %elsebody
  %9 = load i64, ptr %x, align 4
  %10 = icmp slt i64 %9, 10
  br i1 %10, label %whilebody, label %endwhile

whilebody:                                        ; preds = %whilecond
  %11 = load i64, ptr %x, align 4
  %12 = add i64 %11, 1
  store i64 %12, ptr %x, align 4
  br label %whilecond

endwhile:                                         ; preds = %whilecond
  br label %endif

endif:                                            ; preds = %endwhile, %ifbody
  %13 = call i32 (ptr, ...) @printf(ptr @.placeholder.float, ptr %y)
  %14 = call i32 (ptr, ...) @printf(ptr @.placeholder.newline)
  ret void
}
)";
    std::string generated = generator.dump();
    generated = "\n" + generated.substr(generated.find("@.str"));

    ASSERT_EQ(ir, generated);
}
