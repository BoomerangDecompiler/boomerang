<?xml version="1.0" encoding="ISO-8859-1" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:xul="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"	
		version="1.0">

<xsl:strip-space elements="*"/>

<xsl:template match="prog">
<xsl:processing-instruction name="xml-stylesheet">href="chrome://global/skin/" type="text/css" xmlns:html="http://www.w3.org/1999/xhtml" xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"</xsl:processing-instruction>

<xul:window id="cluster-window" title="Clusters and Procedures">
<xul:tree flex="1">
  <xul:treecols>
      <xul:treecol id="cluster" label="Cluster/Procedure" primary="true" flex="3"/>
  </xul:treecols>

  <xul:treechildren>
  <xsl:apply-templates select="cluster"/>
  </xul:treechildren>
  </xul:tree>
  </xul:window>
</xsl:template>

<xsl:template match="cluster">
  <xul:treeitem container="true" open="true">
    <xul:treerow>
      <xul:treecell label="{@name}"/>
    </xul:treerow>
    <xul:treechildren>
      <xsl:apply-templates select="cluster"/>
      <xsl:variable name="id" select="@id"/>
      <xsl:apply-templates select="/prog/userproc[@cluster=$id]"/>
      <xsl:apply-templates select="/prog/libproc[@cluster=$id]"/>
      <xsl:variable name="doc"><xsl:value-of select="@name"/>.xml</xsl:variable>
      <xsl:apply-templates select="document($doc)/procs"/>
    </xul:treechildren>
  </xul:treeitem>
</xsl:template>

<xsl:template match="userproc"><xsl:apply-templates/></xsl:template>
<xsl:template match="libproc"><xsl:apply-templates/></xsl:template>

<xsl:template match="signature">
  <xul:treeitem>
    <xul:treerow>
      <xul:treecell label="{@name}"/>
    </xul:treerow>
  </xul:treeitem>
</xsl:template>

</xsl:stylesheet>

