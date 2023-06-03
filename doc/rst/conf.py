# -*- coding: utf-8 -*-
import sys
import subprocess
import datetime

project = "ampl::ampls-api"
copyright = "2020-{}, AMPL Optimization Inc".format(datetime.date.today().year)
author = "AMPL Inc"

# The full version, including alpha/beta/rc tags
release = "0.1"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ["breathe", "sphinx.ext.graphviz", "myst_parser"]
# Breathe Configuration
breathe_projects = {"ampls-api": "doxyxml"}
breathe_default_project = "ampls-api"
# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = "ampl_sphinx_theme"
html_theme_options = {
    "logo_link": "index",
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/ampl/ampls-api",
            "icon": "fab fa-github",
        },
        {
            "name": "AMPL Resources",
            "url": "https://developers.ampl.com",
            "icon": "fas fa-book fa-fw",
            "target": "_self",
        },
        {
            "name": "AMPL Portal",
            "url": "https://portal.ampl.com",
            "icon": "fas fa-sign-in-alt fa-fw",
            "target": "_self",
        },
        {
            "name": "AMPL.com",
            "url": "https://ampl.com",
            "icon": "fas fa-home fa-fw",
            "target": "_self",
        },
    ],
    "collapse_navigation": True,
    "external_links": [
        # {"name": "Try AMPL", "url": "https://ampl.com"}
    ],
    "logo_text": "Solver API",
}
# html_css_files = [
#     'css/custom.css',
# ]
html_favicon = "_static/cropped-favicon-raw-192x192.png"
# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
html_logo = "_static/logo-inline-web-v4.png"
html_show_sphinx = False
html_show_sourcelink = False


def run_doxygen(folder):
    """Run the doxygen make command in the designated folder"""

    try:
        print("cd %s; doxygen" % folder)
        retcode = subprocess.call("doxygen", cwd=folder, shell=True)
        if retcode < 0:
            sys.stderr.write("doxygen terminated by signal %s" % (-retcode))
    except OSError as e:
        sys.stderr.write("doxygen execution failed: %s" % e)


def generate_doxygen_xml(app):
    """Run the doxygen make commands if we're on the ReadTheDocs server"""
    # read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
    # We always build doxygen documentation
    run_doxygen("./")


def setup(app):
    # Add hook for building doxygen xml
    app.connect("builder-inited", generate_doxygen_xml)
