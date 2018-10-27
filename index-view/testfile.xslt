<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!--
FIXME Replace this poor test file with some more challenging

This file is borrowed from
https://wiki.selfhtml.org/wiki/XML/XSL/XSLT/Sprachelemente
-->

<xsl:template match="/">
 <html>
 <head>
 </head>
 <body>
 <table border="1">
  <xsl:call-template name="Schleife">
    <xsl:with-param name="Zaehler" select="number(/test/start)" />
  </xsl:call-template>
 </table>
 </body>
 </html>
</xsl:template>

<xsl:param name="Foo" /><xsl:variable  name="Baz-.!\"x"/>
<xsl:variable foo-name="Don't match this wrong name attribute"x"/>
<!-- The node is created but empty, it's not a bug, it's to test the RegExp -->

<xsl:template name="Schleife">
 <xsl:param name="Zaehler" />
 <xsl:choose>
  <xsl:when test="$Zaehler &lt;= number(/test/ende)">
    <tr>
      <td><xsl:value-of select="$Zaehler" /></td>
      <td><xsl:value-of select="$Zaehler * $Zaehler" /></td>
    </tr>
    <xsl:call-template name="Schleife">
     <xsl:with-param name="Zaehler" select="$Zaehler + 1" />
    </xsl:call-template>
  </xsl:when>
  <xsl:otherwise>
   <xsl:call-template name="Abbruch" />
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

</xsl:stylesheet>

<!-- The following must not cause trouble -->
<xsl:comment>xsl:template name="Don't show this"</xsl:comment>

<!-- Here is the last tag not finished, but that must not cause a lost template node -->
<xsl:template name="ShowThis"></xsl:template

