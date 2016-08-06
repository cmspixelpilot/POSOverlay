void gaziza(xoap::SOAPElement& e, std::string indent="  ") {
  std::cout << indent << "QualifiedName: " << e.getElementName().getQualifiedName() << "\n";
  std::cout << indent << "TextContent: " << e.getTextContent() << "\n";
  std::cout << indent << "Children:" << std::endl;
  std::vector<xoap::SOAPElement> children = e.getChildElements();
  std::string child_indent = indent + "  ";
  for (size_t i = 0, ie = children.size(); i < ie; ++i)
    gaziza(children[i], child_indent);
}
