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

Some more paragraph

### The third level heading H3
Some paragraph

Some more paragraph

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
paragraph, which conforms to Kate's syntax highliter.

This is a H1 heading
====================

This is a H2 heading
--------------------

On the day of writing this testfile are these Setext-style headers not highlited
by Kate's syntax highliter, but they are allowed by [Gruber][1].

# TODO

Grabbed from GitHub

- [ ] Lists, as these are currently listed as paragraph but could
- [ ] Format code or text into its own distinct block, use triple backticks
- [ ] Quoting code
- [ ] Links
- [ ] Task lists


```
echo Currently is this code block listed as triple backtick paragraph
```

Grabbed from Gruber

- [ ] Horizontal Rules
Images

[1]: https://daringfireball.net/projects/markdown/syntax
[2]: https://help.github.com/articles/basic-writing-and-formatting-syntax
