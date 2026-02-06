// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file main_window.cpp
/// Contains implementation of the main_window class.

#include "main_window.hpp"
#include "../../native_text_view.hpp"
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>

namespace {

using fchar_t = tmvc::wformatted_char;
using ftext_t = tmvc::basic_simple_text_model<fchar_t>;

std::vector<fchar_t> make_formatted(std::wstring_view str,
                                    const tmvc::text_format & fmt = {}) {
    std::vector<fchar_t> out;
    out.reserve(str.size());
    for (auto ch : str) {
        out.emplace_back(ch, fmt);
    }
    return out;
}

void assign_formatted(ftext_t & mdl, std::wstring_view str) {
    tmvc::clear(mdl);
    mdl.insert({0, 0}, make_formatted(str));
}

void assign_formatted(ftext_t & mdl, const std::vector<fchar_t> & chars) {
    tmvc::clear(mdl);
    mdl.insert({0, 0}, chars);
}

std::vector<fchar_t> make_demo_formatted() {
    const std::wstring_view text =
        L"This is\nsimple\ntext\nbla\nbla\nbla\n\nlast line with very long width";

    tmvc::text_format title_fmt{tmvc::color{40, 80, 160},
                                std::nullopt,
                                true,
                                false,
                                false,
                                false};
    tmvc::text_format simple_fmt{tmvc::color{160, 80, 40},
                                 std::nullopt,
                                 false,
                                 true,
                                 false,
                                 false};
    tmvc::text_format text_fmt{tmvc::color{20, 140, 60},
                               std::nullopt,
                               false,
                               false,
                               true,
                               false};
    tmvc::text_format bla_fmt{tmvc::color{120, 0, 140},
                              std::nullopt,
                              false,
                              false,
                              false,
                              true};

    std::vector<fchar_t> out;
    out.reserve(text.size());

    size_t i = 0;
    for (auto ch : text) {
        const tmvc::text_format * fmt = nullptr;
        if (i < 7) { // "This is"
            fmt = &title_fmt;
        } else if (i >= 8 && i < 14) { // "simple"
            fmt = &simple_fmt;
        } else if (i >= 15 && i < 19) { // "text"
            fmt = &text_fmt;
        } else if (i >= 20 && i < 31) { // "bla\nbla\nbla"
            if (ch != L'\n') {
                fmt = &bla_fmt;
            }
        }

        if (fmt) {
            out.emplace_back(ch, *fmt);
        } else {
            out.emplace_back(ch);
        }
        ++i;
    }

    return out;
}

} // namespace


main_window::main_window() {
    assign_formatted(text_, make_demo_formatted());

    auto central_widget = new QWidget{};
    setCentralWidget(central_widget);
    auto layout = new QGridLayout{central_widget};

    auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fnt.setPointSize(14);

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 0, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Rich Std Sel/Controller</b>")});

        using view_t = tmvc::qt::native::text_view <
            text_model_t,
            tmvc::qt::native::std_selection_model,
            tmvc::qt::native::std_edit_controller
        >;

        auto text_view = new view_t{text_};
        text_view->setFont(fnt);
        vlayout->addWidget(text_view);
    }

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 1, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Rich Std Controller</b>")});

        using view_t = tmvc::qt::native::text_view <
            text_model_t,
            selection_model_t,
            tmvc::qt::native::std_edit_controller
        >;

        auto text_view = new view_t{text_, selection_};
        text_view->setFont(fnt);
        vlayout->addWidget(text_view);
    }

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 2, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Rich</b>")});

        using view_t = tmvc::qt::native::text_view <
            text_model_t,
            selection_model_t,
            controller_t
        >;

        auto text_view = new view_t{text_, selection_, controller_};
        text_view->setFont(fnt);
        vlayout->addWidget(text_view);
    }

    auto file_menu = this->menuBar()->addMenu("File");

    auto open_menu = file_menu->addAction("Open...");
    open_menu->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(open_menu, &QAction::triggered, [this] {
        open_file();
    });

    file_menu->addSeparator();

    auto exit_menu = file_menu->addAction("Exit");
    connect(exit_menu, &QAction::triggered, [this] {
        close();
    });
}


void main_window::open_file() {
    auto file_name = QFileDialog::getOpenFileName(this, tr("Select file to open"));
    if (file_name.isEmpty()) {
        return;
    }

    try {
        std::wifstream ifstr{file_name.toStdString(), std::ios::in | std::ios::binary};
        if (!ifstr.is_open()) {
            std::ostringstream msg;
            msg << "Can't open file " << file_name.toStdString() << " for reading";
            throw std::runtime_error{msg.str()};
        }

        // use UTF-8 character converter for system C locale
        std::locale loc("");
        if (loc.name() == "C") {
            loc = std::locale{loc, new std::codecvt_utf8<wchar_t>};
        }

        ifstr.imbue(loc);
        std::wstring wstr;
        while(true) {
            wchar_t ch;
            ifstr.read(&ch, 1);
            if (!ifstr) {
                break;
            }

            wstr.push_back(ch);
        }

        assign_formatted(text_, wstr);
    }
    catch (std::exception & err) {
        QMessageBox msg(QMessageBox::Critical,
                        QObject::tr("Error"),
                        QObject::tr("Error: %1").arg(QString::fromStdString(err.what())),
                        QMessageBox::Ok);
        msg.exec();
    }
}
