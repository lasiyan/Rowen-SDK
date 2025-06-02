#ifndef RAPIDXML_EXT_HPP_INCLUDED
#define RAPIDXML_EXT_HPP_INCLUDED

#include <string>

#include "rapidxml.hpp"

/* Adding declarations to make it compatible with gcc 4.7 and greater */
namespace rapidxml {
namespace internal {

template <class OutIt, class Ch>
inline OutIt print_children(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_attributes(OutIt out, const xml_node<Ch>* node, int flags);

template <class OutIt, class Ch>
inline OutIt print_data_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_cdata_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_element_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_declaration_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_comment_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_doctype_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

template <class OutIt, class Ch>
inline OutIt print_pi_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

}  // namespace internal
}  // namespace rapidxml

template <typename T, class Ch = char>
Ch* allocate_string(rapidxml::xml_document<Ch>* doc, const T& val)
{
  if (doc == nullptr)
    return nullptr;

  if constexpr (std::is_same_v<T, std::string>)
    return doc->allocate_string(val.c_str());
  else if constexpr (std::is_same_v<T, const char*>)
    return doc->allocate_string(val);
  else
    return doc->allocate_string(std::to_string(val).c_str());
}

#include "rapidxml_print.hpp"  // IWYU pragma: export

#endif  // RAPIDXML_EXT_HPP_INCLUDED
