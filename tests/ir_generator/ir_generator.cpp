#include <gtest/gtest.h>
#include <iostream>

#include <backend/lexer/lexer.hpp>
#include <backend/parser/parser.hpp>
#include <backend/semantizer/semantizer.hpp>
#include <ir_generator/ir_generator.hpp>
#include <utils/source_files.hpp>

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

    std::string ir = "; ModuleID = 'module'\n"
                     "source_filename = \"module\"\n"
                     "\n"
                     "@.placeholder.pointer = private unnamed_addr constant [3 x i8] c\"%x\\00\", align 1\n"
                     "@.placeholder.int = private unnamed_addr constant [3 x i8] c\"%d\\00\", align 1\n"
                     "@.placeholder.none = private unnamed_addr constant [5 x i8] c\"None\\00\", align 1\n"
                     "@.placeholder.float = private unnamed_addr constant [3 x i8] c\"%f\\00\", align 1\n"
                     "@.placeholder.str = private unnamed_addr constant [3 x i8] c\"%s\\00\", align 1\n"
                     "@.placeholder.true = private unnamed_addr constant [5 x i8] c\"True\\00\", align 1\n"
                     "@.placeholder.false = private unnamed_addr constant [6 x i8] c\"False\\00\", align 1\n"
                     "@.placeholder.newline = private unnamed_addr constant [2 x i8] c\"\\0A\\00\", align 1\n"
                     "@.str.0 = private unnamed_addr constant [3 x i8] c\"hi\\00\", align 1\n"
                     "\n"
                     "declare i32 @printf(i8*, ...)\n"
                     "\n"
                     "declare i32 @scanf(i8*, ...)\n"
                     "\n"
                     "define double @foo(i64 %0) {\n"
                     "foo_entry:\n"
                     "  %x = alloca i64, align 8\n"
                     "  store i64 %0, i64* %x, align 4\n"
                     "  br label %block\n"
                     "\n"
                     "block:                                            ; preds = %foo_entry\n"
                     "  %1 = load i64, i64* %x, align 4\n"
                     "  %2 = add i64 5, %1\n"
                     "  %z = alloca i64, align 8\n"
                     "  store i64 %2, i64* %z, align 4\n"
                     "  %3 = load i64, i64* %z, align 4\n"
                     "  %typeconv = sitofp i64 %3 to double\n"
                     "  %4 = fadd double 3.500000e+00, %typeconv\n"
                     "  ret double %4\n"
                     "}\n"
                     "\n"
                     "define void @main() {\n"
                     "main_entry:\n"
                     "  br label %block\n"
                     "\n"
                     "block:                                            ; preds = %main_entry\n"
                     "  %x = alloca i64, align 8\n"
                     "  store i64 1, i64* %x, align 4\n"
                     "  %y = alloca double, align 8\n"
                     "  %input = alloca double, align 8\n"
                     "  %0 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* "
                     "@.placeholder.float, i32 0, i32 0), double* %input)\n"
                     "  %1 = load double, double* %input, align 8\n"
                     "  store double %1, double* %y, align 8\n"
                     "  %2 = load [3 x i8], [3 x i8]* @.str.0, align 1\n"
                     "  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* "
                     "@.placeholder.str, i32 0, i32 0), [3 x i8] %2)\n"
                     "  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* "
                     "@.placeholder.newline, i32 0, i32 0))\n"
                     "  br label %ifcond\n"
                     "\n"
                     "ifcond:                                           ; preds = %block\n"
                     "  %5 = load double, double* %y, align 8\n"
                     "  %6 = fcmp ogt double %5, 1.500000e+00\n"
                     "  br i1 %6, label %ifbody, label %elsebody\n"
                     "\n"
                     "ifbody:                                           ; preds = %ifcond\n"
                     "  %7 = load i64, i64* %x, align 4\n"
                     "  %8 = call double @foo(i64 %7)\n"
                     "  %9 = fmul double %8, 2.500000e+00\n"
                     "  %z = alloca double, align 8\n"
                     "  store double %9, double* %z, align 8\n"
                     "  br label %endif\n"
                     "\n"
                     "elsebody:                                         ; preds = %ifcond\n"
                     "  %typeconv = sitofp i64 6 to double\n"
                     "  store double %typeconv, double* %y, align 8\n"
                     "  br label %whilecond\n"
                     "\n"
                     "whilecond:                                        ; preds = %whilebody, %elsebody\n"
                     "  %10 = load i64, i64* %x, align 4\n"
                     "  %11 = icmp slt i64 %10, 10\n"
                     "  br i1 %11, label %whilebody, label %endwhile\n"
                     "\n"
                     "whilebody:                                        ; preds = %whilecond\n"
                     "  %12 = load i64, i64* %x, align 4\n"
                     "  %13 = add i64 %12, 1\n"
                     "  store i64 %13, i64* %x, align 4\n"
                     "  br label %whilecond\n"
                     "\n"
                     "endwhile:                                         ; preds = %whilecond\n"
                     "  br label %endif\n"
                     "\n"
                     "endif:                                            ; preds = %endwhile, %ifbody\n"
                     "  %14 = load double, double* %y, align 8\n"
                     "  %15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* "
                     "@.placeholder.float, i32 0, i32 0), double %14)\n"
                     "  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* "
                     "@.placeholder.newline, i32 0, i32 0))\n"
                     "  ret void\n"
                     "}\n";
    ASSERT_EQ(ir, generator.dump());
}
