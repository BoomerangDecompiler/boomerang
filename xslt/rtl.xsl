<?xml version="1.0" encoding="ISO-8859-1" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:html="http://www.w3.org/1999/xhtml"
		version="1.0">

<xsl:strip-space elements="*"/>
<xsl:output method="html"/>

<xsl:template match="/procs">
    <html>
	<body>
	  <xsl:apply-templates/>
	</body>
    </html>
</xsl:template>


<xsl:template match="//userproc">
  <xsl:apply-templates select="./signature"/>
  <xsl:apply-templates select="./cfg"/>
</xsl:template>

<xsl:template match="signature">
  <br/><br/><xsl:apply-templates select="./return[1]/type"/><xsl:text> </xsl:text><xsl:value-of select="@name"/>(<xsl:apply-templates select="./param"/> implicit: <xsl:apply-templates select="./implicitparam"/>) { <xsl:apply-templates select="./return"/> }
</xsl:template>

<xsl:template match="param"><xsl:apply-templates select="./type/*"/><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:text> </xsl:text><xsl:apply-templates select="./exp/*"/>, </xsl:template>
<xsl:template match="implicitparam"><xsl:apply-templates select="./type/*"/><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:text> </xsl:text><xsl:apply-templates select="./exp/*"/>, </xsl:template>
<xsl:template match="return"><xsl:apply-templates select="./exp/*"/>, </xsl:template>

<xsl:template match="integertype">int</xsl:template>
<xsl:template match="pointertype"><xsl:apply-templates/>*</xsl:template>
<xsl:template match="chartype">char</xsl:template>

<xsl:template match="cfg">
  <xsl:apply-templates select="./bb"/>
</xsl:template>

<xsl:template match="bb[@nodeType='3']">
<br/><br/>CallBB:<xsl:apply-templates select="./rtl/stmt/*"/>
</xsl:template>

<xsl:template match="bb[@nodeType='4']">
<br/><br/>RetBB:<xsl:apply-templates select="./rtl/stmt/*"/>
</xsl:template>

<xsl:template match="bb">
<br/><br/><xsl:apply-templates select="./rtl/stmt/*"/>
</xsl:template>

<xsl:template match="assign">
  <br/><xsl:value-of select="@number"/> *<xsl:value-of select="./type/*/@size"/>* <xsl:apply-templates select="./lhs/*"/> := <xsl:apply-templates select="./rhs/*"/> 
</xsl:template>

<xsl:template match="callstmt">
  <xsl:variable name="proc" select="./dest/@proc"/>
  <br/><xsl:value-of select="@number"/> CALL <xsl:value-of select="//*[@id=$proc]/signature/@name"/>(implicit: <xsl:apply-templates select="./implicitarg"/>) { <xsl:apply-templates select="./returnexp"/> }
</xsl:template>

<xsl:template match="returnstmt">
  <br/><xsl:value-of select="@number"/> RET
</xsl:template>

<xsl:template match="implicitarg"><xsl:apply-templates/>, </xsl:template>

<xsl:template match="returnexp"><xsl:apply-templates/>, </xsl:template>

<xsl:template match="location[@op='opRegOf']">r<xsl:value-of select="./subexp1/const/@value"/></xsl:template>

<xsl:template match="terminal[@op='opFlags']">%flags</xsl:template>
<xsl:template match="terminal[@op='opPC']">%pc</xsl:template>

<xsl:template match="location[@op='opMemOf']">
  m[<xsl:apply-templates select="./subexp1/*"/>]
</xsl:template>

<xsl:template match="location[@op='opTemp']">
  <xsl:value-of select="./subexp1/const/@value"/>
</xsl:template>

<xsl:template match="const">
  <xsl:value-of select="@value"/>
</xsl:template>

<xsl:template match="binary[@op='opPlus']">
  <xsl:apply-templates select="./subexp1/*"/> + <xsl:apply-templates select="./subexp2/*"/>
</xsl:template>

<xsl:template match="binary[@op='opMinus']">
  <xsl:apply-templates select="./subexp1/*"/> - <xsl:apply-templates select="./subexp2/*"/>
</xsl:template>

<xsl:template match="binary[@op='opFlagCall']">
  <xsl:value-of select="./subexp1/const/@value"/>(<xsl:apply-templates select="./subexp2/*"/>)
</xsl:template>

<xsl:template match="binary[@op='opList']">
  <xsl:apply-templates select="./subexp1/*"/>, <xsl:apply-templates select="./subexp2/*"/> 
</xsl:template>

<xsl:template match="terminal[@op='opNil']"></xsl:template>

</xsl:stylesheet>

