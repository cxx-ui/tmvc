// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file main_window.cpp
/// Contains implementation of the main_window class.

#include "main_window.hpp"
#include "../../text_view.hpp"
#include "../../label_text_view.hpp"
#include "../../plain_text_view.hpp"
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <memory>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>


main_window::main_window() {
    auto initial_text = L"This is\nsimple\ntext\nbla\nbla\nbla\n\nlast line with very long width";
    assign(text_, initial_text);

    auto central_widget = new QWidget{};
    setCentralWidget(central_widget);
    auto layout = new QGridLayout{central_widget};

    auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fnt.setPointSize(14);

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 0, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Label Text View</b>")});

        auto label_view = new tmvc::qt::label_text_view(text_);
        label_view->setFont(fnt);
        label_view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        vlayout->addWidget(label_view, 1);
    }

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 1, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Plain Text View</b>")});

        auto text_view = new tmvc::qt::plain_text_view<tmvc::wsimple_text_model>{text_};
        text_view->setWordWrapMode(QTextOption::NoWrap);
        text_view->setFont(fnt);
        vlayout->addWidget(text_view);
    }

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 2, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Plain Selectable Text View</b>")});

        using view_t = tmvc::qt::plain_selectable_text_view<text_model_t>;
        auto text_view = new view_t{text_, plain_selection_};
        text_view->setFont(fnt);
        text_view->setWordWrapMode(QTextOption::NoWrap);
        vlayout->addWidget(text_view);
    }

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 0, 3, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Plain Controllable View</b>")});

        using view_t = tmvc::qt::plain_controllable_text_view <
            text_model_t,
            plain_controller_t
        >;
        auto text_view = new view_t{text_, plain_selection_, plain_controller_};
        text_view->setFont(fnt);
        text_view->setWordWrapMode(QTextOption::NoWrap);
        vlayout->addWidget(text_view);
    }

    {
        auto vlayout = new QVBoxLayout;
        layout->addLayout(vlayout, 1, 3, 1, 1);

        vlayout->addWidget(new QLabel{tr("<b>Text View (advanced)</b>")});

        auto text_viewer = new tmvc::qt::text_view{text_, selection_, viewport_pos_, controller_};
        text_viewer->setFont(fnt);
        vlayout->addWidget(text_viewer);
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

        tmvc::assign(text_, wstr);
    }
    catch (std::exception & err) {
        QMessageBox msg(QMessageBox::Critical,
                        QObject::tr("Error"),
                        QObject::tr("Error: %1").arg(QString::fromStdString(err.what())),
                        QMessageBox::Ok);
        msg.exec();
    }
}
