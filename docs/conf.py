# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

from datetime import datetime

current_year = datetime.now().year

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Compiler Project'
copyright = f'2020-{current_year}, vla5924-practice'
author = 'vla5924-practice'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_rtd_theme',
    'myst_parser',
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_show_sourcelink = False
html_sidebars = { '**': ['globaltoc.html', 'relations.html', 'sourcelink.html', 'searchbox.html'] }
html_style = 'css/custom.css'
html_context = {
    "display_github": True,
    "github_user": "vla5924-practice",
    "github_repo": "compiler-project",
    "github_version": "main",
    "conf_py_path": "/docs/",
}
