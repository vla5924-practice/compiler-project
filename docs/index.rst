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
   :caption: О проекте

   overview/README.md
   language/README.md

.. toctree::
   :caption: Внутренние представления

   representations/ast/README.md
   representations/optree/README.md


.. toctree::
   :caption: Компоненты проекта

   frontend/README.md
   backend_ast/README.md
   backend_optree/README.md
   codegen/README.md
   cli/README.md


.. toctree::
   :caption: Сравнения производительности

   benchmarking/compilation/README.md
   benchmarking/execution/README.md


.. toctree::
   :caption: См. также

   references.md
   repository.md

..

   «Таким образом, был разработан и реализован компилятор».
