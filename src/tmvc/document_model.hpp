// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file document_model.hpp
/// Contains definition of the document_model concept.

#pragma once

#include "line_numbers_model.hpp"
#include "text_model.hpp"
#include "selection_model.hpp"
#include "selection_controller.hpp"
#include "viewport_position_model.hpp"
#include <tuple>
#include <type_traits>


namespace tmvc {


/// Represents document section. Stores references to text model, selection model,
/// and controller.
template <
    text_model TextModel,
    selection_model SelectionModel,
    selection_controller Controller
>
class document_section {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of controller
    using controller_t = Controller;

    /// Constructs section with specified references to its components
    document_section(text_model_t & txt, selection_model_t & sel, controller_t & cntrl):
        text_{txt}, selection_{sel}, controller_{cntrl} {}

    /// Returns reference to text model
    auto & text() { return text_; }

    /// Returns reference to selection model
    auto & selection() { return selection_; }

    /// Returns reference to controller
    auto & controller() { return controller_; }

private:
    text_model_t & text_;               ///< Reference to text model
    selection_model_t & selection_;     ///< Reference to selection model
    controller_t & controller_;         ///< Reference to controller
};


/// True if type is an instance of document_section template
template <typename T>
constexpr bool is_document_section = false;
template <typename TextModel, typename SelectionModel, typename Controller>
constexpr bool is_document_section<document_section<TextModel, SelectionModel, Controller>> = true;


/// True if types are instances of document_section template 
template <typename ... Types>
constexpr inline bool is_document_section_sequence = false;
template <>
constexpr inline bool is_document_section_sequence<> = true;
template <typename Head, typename ... Tail>
constexpr bool is_document_section_sequence<Head, Tail...> =
    is_document_section<Head> && is_document_section_sequence<Tail...>;


/// True if type is a tuple of document_section templates
template <typename Tuple>
constexpr bool is_document_section_tuple = false;
template <typename ... Sections>
constexpr bool is_document_section_tuple<std::tuple<Sections...>> =
    is_document_section_sequence<Sections...> &&
    std::tuple_size_v<std::tuple<Sections...>> > 0;


/// Represents document that contains multiple sections with single text view
/// model in each section. All text view models have single synchronized viewport
/// position model.
template <typename DocumentModel>
concept document_model = requires(DocumentModel & doc) {
    /// Returns reference to line numbers model
    requires std::is_lvalue_reference_v<decltype(doc.line_numbers())>;
    requires line_numbers_model<std::remove_cvref_t<decltype(doc.line_numbers())>>;

    /// Returns tuple of text view models for all document sections
    requires is_document_section_tuple<std::remove_cvref_t<decltype(doc.sections())>>;

    /// Returns reference to viewport position model for all sections
    requires std::is_lvalue_reference_v<decltype(doc.viewport_pos())>;
    requires viewport_position_model<std::remove_cvref_t<decltype(doc.viewport_pos())>>;
};


/// Type of viewport position model in document model
template <document_model DocumentModel>
using document_model_viewport_pos_t = std::remove_cvref_t<decltype(std::declval<DocumentModel>().viewport_pos())>;


/// Type of tuple of sections of document model
template <document_model DocumentModel>
using document_model_sections_t = std::remove_cvref_t<decltype(std::declval<DocumentModel>().sections())>;


/// Number of sections in document model
template <document_model DocumentModel>
constexpr inline size_t document_model_sections_count_v = std::tuple_size_v<document_model_sections_t<DocumentModel>>;


/// Type of section of document model at specified index
template <document_model DocumentModel, size_t idx>
using document_model_section_t = std::tuple_element_t<idx, document_model_sections_t<DocumentModel>>;


/// Type of text model type of document section at specified index
template <document_model DocumentModel, size_t idx>
using document_model_section_text_model_t = typename document_model_section_t<DocumentModel, idx>::text_model_t;


/// Type of selection model type of document section at specified index
template <document_model DocumentModel, size_t idx>
using document_model_section_selection_model_t = typename document_model_section_t<DocumentModel, idx>::selection_model_t;


/// Type of controller type of document section at specified index
template <document_model DocumentModel, size_t idx>
using document_model_section_controller_t = typename document_model_section_t<DocumentModel, idx>::controller_t;


}
