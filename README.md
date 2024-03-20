## KatePlugin-IndexView

This is a fork from Kate's plugin *SymbolViewer* done 2018 with a couple of
modifications.

  - Support for text files like Markdown
  - Support for XML files like HTML or DocBook
  - Show *FIXME/TODO/BEGIN* notes found in the current file
  - Has a *filter field* to filter the tree by pattern and currently selected text
  - Save settings individual for each file type
  - Keep the editing responsive when the file is large

In July 2018 I started to hack on these plugin and got some of may changes
upstream which was a very cool experience, but not without some hassle. Lastly I
was tired to fight for each small change and saw no chance to get forward this
way. So I decide to duck away and do it silently, which took longer as I
thought.

Now, 2022, Kate has the LSP plugin which offer a similar view like this plugin,
or even the *SymbolViewer*, which make this kind of plugin somehow obsolete ...
as long as you like the new fancy stuff and have a fitting LSP server installed
for your file in use.

The last two points and the fact that I got used to work with my plugin, made me
spend time again to ensure it will work in the future too. Well, there is more
to do to make it ready for the future but some steps are done.


### Last version is 0.9.2, Mar 2024

  - Keep it working with recent Kate version 24.02
  - CMake changes to compile with Qt6/KF6, sadly not perfect
  - Code requires now Qt 6.6

Older release notes can you find below.


## Installation

### Packages

Arch Linux has an AUR, search for *kate-indexview-plugin*.


### From Source

> [!NOTE]
> The build/cmake system is currently in bad condition, any help is welcome!
> To install use instead something like this...
>
>     $ sudo install -Dm755  index-view/kateindexviewplugin.so /usr/lib/qt6/plugins/kf6/ktexteditor/
>
> ...and not "make install" as mentioned below.

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


## Post Install Task

Restart Kate and activate in the Plugin Manager *Index View*.

Hint: In German it's named *Stichwortansicht*.


## License

KatePlugin-IndexView is licensed under LGPL2. See COPYING for the full license.


<!-- CAUTION! Trailing two spaces are there to have a line break without new paragraph -->
KatePlugin-IndexView  
Copyright (C) 2018, 2019, 2022, 2024 loh.tar@googlemail.com

Because KatePlugin-IndexView is a fork there are rights from pervious main
developers:

<!-- CAUTION! Trailing two spaces are there to have a line break without new paragraph -->
plugin_katesymbolviewer  
Copyright (C) 2014 Kåre Särs <kare.sars@iki.fi>  
Copyright (C) 2003 Massimo Callegari <massimocallegari@yahoo.it>

There are others who contribute special parsers, see source files for details,
and probably many more I didn't have knowledge from. Thanks to all!


## Release History

### 0.9.1, Aug 2022

  - Add AsciiDocParser with minimal support, but can be improved if needed
  - Fix flicker when edit at the end of a (e.g. Markdown) document


### 0.9, Jul 2022

  - Prepare for Qt6 and port away from QRegExp, Required Qt version is now 5.15
  - Some parser have now less options to show/hide some kind of node type, but
    new options to limit the shown nesting
  - New parser master class XmlTypeParser which can support any kind of XML file
    with very less extra effort. Replaces the old XsltParser and support atm
    additional HTML, SGML/Docbook, XML and DTD. All in different
    states of usefulness/completeness
  - New support for INI and Diff files
  - CppParser: Don't group name spaces/structs below one such root node but
    create a root node with the name space name
  - Parser: Keep the editing responsive when the file is large
  - Parser: Use BashParser for "Zsh" files
  - Replace the cute good old xmp icons with own auto generated icons
  - IndexView: Try to optimize item update to reduce pointless changes, but it
    works some times unexpected/not optimal, sorry
  - Cache for each doc the tree and parser. This makes doc switching delay free
    and avoid as best as possible any unpleasant flickering when editing
  - Add "Outline" to the plugin description to accommodate the apparently
    established terms
  - Many more small fixes and bigger changes, see commit log for detailed list


### 0.8.4, May 2019

  - CppParser: Group function definitions by class name
  - CppParser: Fix recognition of functions with a reference as return type
  - MarkdownParser: Fix false detection of e.g. headings in indented blocks
  - Parser: Add view option to remove the cute icons
  - Parser: Fix rare cases with a broken save&restore of settings
  - ProgramParser: Add BEGIN tag recognition like FIXME/TODO


### 0.8.3, Dez 2018

  - FortranParser: Fix crash when line is empty


### 0.8.2, Nov 2018

  - CppParser: Fix function recognition with Multiple::Name::Space
  - CppParser: Fix function recognition with a return type like Foo<Bar>
  - ProgramParser: Fix crash in nesting logic


### 0.8.1, Okt 2018

  - DocumentParser: Fix crash in some cases


### Very first version was 0.8, Okt 2018

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

