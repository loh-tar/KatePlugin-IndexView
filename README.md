KatePlugin-IndexView
======================

This is a fork from Kate's plugin *SymbolViewer* with a couple of improvements.

In July 2018 I started to hack on these plugin and got some of may changes
upstream which was a very cool experience, but not without some hassle.

Lastly I was tired to fight for each small change and saw no chance to get
forward this way. So I decide to duck away and do it silently, which take longer
as I thought. I'm sure it was the right decision, because freed from
considerations there was so many and large changes, with some Try & Error, that
it had wouldn't make sense with upstream patches.

Now, I offer it as an independent plugin. We will see if it find a way into the
official Kate version.


Last (and very first) version is 0.8, Okt 2018
------------------------------------------------

  - Support Markdown files
  - Support plain text files
  - Display *FIXME/TODO* notes as detached own top level tree node. There is no
    context menu option to disable these. Furthermore will the *Tree View*
    option ignored. The purpose is to have always a quick point to issues
  - Add a *filter field* with a history saved on session basis. It's independent
    from the current document and limited to 9 entries. The filter is a simple,
    case insensitive pattern without space
  - Add *smart filtering* due to currently selected text. Filters only when
    there are at least three char selected and there is a match found. This kind
    of filtering has precedence over the pattern in the *filter field*
  - Add a global option to expand the nodes by a *cozy click*. It's like a smart
    single click. Most top level items are toggled on first click all other on
    second click. But after this second click they toggle on each click
  - Improve/Fix existing parser due to complete rewrite. But mostly is now the
    *view parameters* option gone because I'm lazy and not full convinced how
    important these are. Some parser may still need more love than others to be
    considered *finished*. For example the ecma (JavaScript) one where I was too
    confused how to name which kind of function
  - Add parser classes which allow to write new parser more easily. Well, there
    is also the idea to base such functionality on Kate's syntax highlighter
    which should reduce (or avoid) the need to write special parser for each
    kind of file
  - Add an about tab to the config page with version info not only for the plugin
    but also for each parser. The version numbers are chosen by a gut feeling
  - Avoid annoying flicker of the tree while editing
  - Remove annoying tool tips
  - Allow individual options in the context menu for each parser
  - Remove dynamically useless options from the context menu
  - Save settings individual for each parser


Installation
==============

From Source
-------------

In any doubt or error message while you try to compile this plugin, it may help
to look at Kate's home and there [build instructions](https://kate-editor.org/build-it/).
There is listed which package you may need.

The usual way to compile and install is...

    $ cd <source-tree>
    $ mkdir build; cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
    $ make
    $ sudo make install

...however, I noticed that not always the plugin will installed to the right place.

The solution should be to add to the `cmake` line `-DKDE_INSTALL_USE_QT_SYS_PATHS=ON`

So take a look where your Kate plugins are installed:

    $ find /usr/ -name katesymbolviewerplugin.so 2>/dev/null

If it looks like `/usr/lib/qt/plugins/..` add the extra -D switch, if it looks
like `/usr/lib64/plugins/..` don't.

If it's not shown in Kate's Plugin Manager look where it is gone...

    $ find /usr/ -name kateindexviewplugin.so 2>/dev/null

...and move it to the right place.

Non Unices user has do something similar.


Packages
----------

Arch Linux has an AUR, search for *kate-indexview-plugin*.


Post Install Task
===================

Restart Kate and activate in the Plugin Manager *Index View*.

Hint: In German it's named *Stichwortansicht*.


License
=========

KatePlugin-IndexView is licensed under LGPL2. See COPYING for the full license.


<!-- CAUTION! Trailing two spaces are there to have a line break without new paragraph -->
KatePlugin-IndexView  
Copyright (C) 2018 loh.tar@googlemail.com

Because KatePlugin-IndexView is a fork there are rights from pervious main
developers:

<!-- CAUTION! Trailing two spaces are there to have a line break without new paragraph -->
plugin_katesymbolviewer  
Copyright (C) 2014 Kåre Särs <kare.sars@iki.fi>  
Copyright (C) 2003 Massimo Callegari <massimocallegari@yahoo.it>

There are others who contribute special parsers, see source files for details,
and probably many more I didn't have knowledge from. Thanks to all!
