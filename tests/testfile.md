# Test And Demo File For The MarkdownParser
There are two main sources of reference documentation for the Markdown syntax,
these by [John Gruber][1], the inventor of Markdown, and these by [GitHub][2]
which is probably the most used one.

Our MarkdownParser tries to follow both of them where possible, but in case of
a conflict I tend to follow the GitHub version.

Currently are only rudimentary things supported, like headers and paragraphs,
whereby everything which is not a header is treated as paragraph.

# The largest heading H1
## The second largest heading H2
Some paragraph

```foo
echo Code blocks are ignored in general, therefore

# is this no heading

or anything else shown
```
Some more paragraph

### The third level heading H3
Some paragraph

Some more paragraph

Regarding HTML tags wrote [Gruber][1]

<pre>
      The only restrictions are that block-level HTML elements
      — e.g. <div>, <table>, <pre>, <p>, etc. -
      must be separated from surrounding content by blank lines,
      and the start and end tags of the block should not be indented with tabs or spaces.
      Markdown is smart enough not to add extra (unwanted) <p> tags around HTML block-level tags.
</pre>

This <pre> Block is never shown, just like the Code block above

#### The forth heading H4
Some paragraph

Some more paragraph

##### The fifths heading H5
Some paragraph

Some more paragraph

###### The smallest heading H6
Some paragraph

Some more paragraph

####### More sharps than six are not treated as header, therefore is this a
paragraph, which conforms to Kate's syntax highlighter.

This is a H1 heading
====================

This is a H2 heading
--------------------

On the day of writing this test file are these Setext-style headers not
highlighted by Kate's syntax highlighter, but they are allowed by [Gruber][1].

# TODO

## Grabbed from GitHub

- [ ] Lists (with or without Task feature) as these are currently listed as
      paragraph but could become an own node type
- [ ] Links could be collected like FIXME/TODO


## Grabbed from Gruber

- [ ] Horizontal Rules
Images

[1]: https://daringfireball.net/projects/markdown/syntax
[2]: https://help.github.com/articles/basic-writing-and-formatting-syntax
