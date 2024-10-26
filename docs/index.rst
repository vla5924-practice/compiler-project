##################
  Compiler Project
##################


.. image:: https://github.com/vla5924-practice/compiler-project/actions/workflows/ci.yml/badge.svg?branch=main&event=push
    :target: https://github.com/vla5924-practice/compiler-project/actions/workflows/ci.yml

.. image:: https://github.com/vla5924-practice/compiler-project/actions/workflows/devcontainer.yml/badge.svg?branch=main&event=push
   :target: https://github.com/vla5924-practice/compiler-project/actions/workflows/devcontainer.yml

.. image:: https://github.com/vla5924-practice/compiler-project/actions/workflows/codeql.yml/badge.svg?branch=main&event=push
   :target: https://github.com/vla5924-practice/compiler-project/actions/workflows/codeql.yml

.. image:: https://coveralls.io/repos/github/vla5924-practice/compiler-project/badge.svg?branch=main
   :target: https://coveralls.io/github/vla5924-practice/compiler-project?branch=main

.. image:: https://www.bestpractices.dev/projects/6129/badge
   :target: https://www.bestpractices.dev/projects/6129


This is a home for `Compiler project <https://github.com/vla5924-practice/compiler-project>`_, a free for use, open
source, written from scratch compiler with additional support of LLVM IR code generation and binary executable
production. Use links below to dive deep into the product understanding.


.. toctree::
   :caption: Documentation
   :maxdepth: 2

   Документация на русском языке <ru/README>
   Documentation in English <en/README>


.. toctree::
   :caption: Общая информация о проекте
   :hidden:

   ru/overview/architecture
   ru/overview/stages
   ru/overview/subject_area


.. toctree::
   :caption: Компилируемый язык
   :hidden:

   Обзор <ru/language/README>
   ru/language/blocks
   ru/language/conditions
   ru/language/expressions
   ru/language/functions
   ru/language/identifiers
   ru/language/keywords
   ru/language/loops
   ru/language/operators
   ru/language/specials
   ru/language/types
   ru/language/variables


.. toctree::
   :caption: Руководства
   :hidden:

   ru/tutorials/install
   ru/tutorials/llvm


.. toctree::
   :caption: Компоненты (модули) проекта
   :hidden:

   Обзор <ru/components/README>
   ru/components/preprocessor/README
   ru/components/lexer/README
   ru/components/parser/README
   ru/components/semantizer/README
   ru/components/optimizer/README
   ru/components/ast/README
   ru/components/ir_generator/README
   ru/components/cli/README
