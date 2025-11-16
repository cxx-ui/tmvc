// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file document_view.hpp
/// Contains definition of the document_view class.

#pragma once

#include "document_view_splitter.hpp"
#include "text_view.hpp"
#include "../empty_selection_controller.hpp"
#include "../empty_selection_model.hpp"
#include "../document_model.hpp"
#include "../line_numbers_text_model.hpp"
#include <QHBoxLayout>
#include <QSplitter>
#include <tuple>


namespace tmvc::qt {


namespace impl {
    /// Type of text view for section of document model
    template <document_model DocumentModel, size_t idx>
    using document_model_section_view_t = text_view <
        document_model_section_text_model_t<DocumentModel, idx>,
        document_model_section_selection_model_t<DocumentModel, idx>,
        document_model_viewport_pos_t<DocumentModel>,
        document_model_section_controller_t<DocumentModel, idx>
    >;

    /// Makes tuple of pointers to section views from index sequence
    template <document_model DocumentModel, typename IndexSequence>
    struct document_model_section_view_tuple_from_index_sequence;

    template <typename DocumentModel, size_t ... Indexes>
    struct document_model_section_view_tuple_from_index_sequence<DocumentModel, std::index_sequence<Indexes...>> {
        using type = std::tuple<document_model_section_view_t<DocumentModel, Indexes> * ...>;
    };

    template <typename DocumentModel, typename IndexSequence>
    using document_model_section_view_tuple_from_index_sequence_t =
        typename document_model_section_view_tuple_from_index_sequence<DocumentModel, IndexSequence>::type;

    /// Type of tuple of pointers to section views for document model
    template <document_model DocumentModel>
    using document_model_section_view_tuple = document_model_section_view_tuple_from_index_sequence_t <
        DocumentModel,
        typename std::make_index_sequence<document_model_sections_count_v<DocumentModel>>
    >;
}


/// Document view. Displays text view for each section of document.
template <document_model DocumentModel>
class document_view: public QFrame {
public:
    /// Type of view for section of document
    template <size_t Index>
    using section_view_t = impl::document_model_section_view_t<DocumentModel, Index>;

    /// Type of tuple of pointers to views for all sections
    using section_view_tuple = impl::document_model_section_view_tuple<DocumentModel>;

    /// Number of sections in document and view
    static constexpr auto sections_count_v = document_model_sections_count_v<DocumentModel>;

    /// Constructs view with specified reference to document and pointer to parent widget
    document_view(DocumentModel & doc, QWidget * parent = nullptr):
    QFrame{parent},
    doc_{doc},
    line_numbers_text_{std::get<0>(doc_.sections()).text(), doc_.line_numbers()} {
        // creating layout
        auto layout = new QHBoxLayout{this};
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // adding view for line numbers
        line_numbers_ = new line_numbers_view_t{line_numbers_text_,
                                                line_numbers_selection_,
                                                doc_.viewport_pos(),
                                                line_numbers_controller_};
        layout->addWidget(line_numbers_);
        line_numbers_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        line_numbers_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        line_numbers_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        line_numbers_->viewport()->setBackgroundRole(QPalette::AlternateBase);
        line_numbers_->setFrameStyle(QFrame::NoFrame);
        line_numbers_->viewport()->setCursor(Qt::ArrowCursor);

        // adding splitter for sections
        splitter_ = new document_view_splitter;
        layout->addWidget(splitter_);

        // adding views for all sections
        [this]<size_t ... Indexes>(const std::index_sequence<Indexes...> &) {
            (add_section<Indexes>(splitter_), ...);
        }(std::make_index_sequence<sections_count_v>());

        // setting frame style
        this->setFrameStyle(QFrame::StyledPanel);
    }

    /// Returns tuple of section views
    section_view_tuple sections() {
        return sections_;
    }

    /// Returns pointer to view for section at specified index
    template <size_t Index>
    section_view_t<Index> * section() {
        return std::get<Index>(sections_);
    }

    /// Returns pointer to line numbers view widget
    auto line_numbers() {
        return line_numbers_;
    }

    /// Returns pointer to splitter widget used to split sections
    QSplitter * splitter() {
        return splitter_;
    }


protected:
    /// Handles widget property change events
    void changeEvent(QEvent * event) override {
        // setting left and right margin to size of widest font character

        auto margin = default_text_margin();

        // updating margins for line numbers view
        line_numbers_->set_left_margin(margin);
        line_numbers_->set_right_margin(margin);
        line_numbers_->updateGeometry();

        // updating margins for all sections
        [this, margin]<size_t ... Indexes>(const std::index_sequence<Indexes...> &) {
            (update_section_margin<Indexes>(margin), ...);
        }(std::make_index_sequence<sections_count_v>());
    }


private:
    /// Adds sections into layout starting from specified index
    template <size_t Index = 0>
    void add_sections(QSplitter * splitter) {
        constexpr auto is_last = (Index == std::tuple_size_v<section_view_tuple> - 1);
        auto view = add_section_view(splitter, std::get<Index>(doc_.sections()), is_last);
        std::get<Index>(sections_) = view;

        if constexpr (!is_last) {
            add_sections<Index + 1>(splitter);
        }
    }

    /// Adds view for single section into layout
    template <size_t Index>
    void add_section(QSplitter * splitter) {
        constexpr auto is_last = Index == sections_count_v - 1;

        auto sects = doc_.sections();
        auto & sect = std::get<Index>(sects);
        auto view = new text_view{sect.text(),
                                  sect.selection(),
                                  doc_.viewport_pos(),
                                  sect.controller()};
        splitter->addWidget(view);
        view->setFrameStyle(QFrame::NoFrame);
        auto margin = default_text_margin();
        view->set_left_margin(margin);
        view->set_right_margin(margin);

        // hiding vertical scroll bar for all sections except the last one
        if constexpr (!is_last) {
            view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

        // Setting stretch to 1 for the last section. With this, the splitter
        // will resize only the last section when resizing.
        if constexpr (is_last) {
            splitter->setStretchFactor(Index, 1);
        }

        // saving view in tuple of views
        std::get<Index>(sections_) = view;
    }

    /// Update left and right margins of section view
    template<size_t Index>
    void update_section_margin(int margin) {
        section<Index>()->set_left_margin(margin);
        section<Index>()->set_right_margin(margin);
    }

    /// Returns default text margin for sections
    int default_text_margin() const {
        QFontMetrics metrics{font()};
        return metrics.horizontalAdvance("0");
    }


    /// Type of line numbers model
    using line_numbers_model_t = std::remove_cvref_t<decltype(std::declval<DocumentModel>().line_numbers())>;

    /// Type of main document section
    using main_section_t = std::tuple_element_t <
        0,
        std::remove_cvref_t<decltype(std::declval<DocumentModel>().sections())>
    >;

    /// Type of main section text model
    using main_section_text_model_t = typename main_section_t::text_model_t;

    /// Type of line numbers text model
    using line_numbers_text_model_t =  line_numbers_text_model <
        main_section_text_model_t,
        line_numbers_model_t
    >;

    /// Type of line numbers text view
    using line_numbers_view_t = text_view <
        line_numbers_text_model_t,
        empty_selection_model,
        std::remove_cvref_t<decltype(std::declval<DocumentModel>().viewport_pos())>,
        empty_selection_controller<typename line_numbers_text_model_t::char_t>
    >;

    DocumentModel & doc_;                           ///< Reference to document model
    line_numbers_text_model_t line_numbers_text_;   ///< Line numbers text model
    empty_selection_model line_numbers_selection_;  ///< Line numbers selection model

    /// Line numbers controller
    empty_selection_controller<typename line_numbers_text_model_t::char_t> line_numbers_controller_;

    QSplitter * splitter_;                          ///< Splitter widget used to display sections
    line_numbers_view_t * line_numbers_;            ///< Pointer to line numbers view
    section_view_tuple sections_;                   ///< Tuple of pointers to views for all sections
};


}
