#include <imgui.h>
#include <imgui-SFML.h>

#include <TextEditor.h>
#include <ImGuiFileDialog.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <fstream>

constexpr const char* fileTypeFilter = "Source files (*.cpp *.h *.hpp *.txt){.cpp,.h,.hpp,.txt}";

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "EditorTest");
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);

    // Since we only draw ImGui stuff atm we need this
    window.resetGLStates();
    sf::Clock deltaClock;

    TextEditor editor;
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    editor.SetShowWhitespaces(false);
    std::string fileName = "Untitled";

    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type)
            {
            case sf::Event::Closed: window.close(); break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::Escape: window.close(); break;
                default: break;
                }
            default: break;
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        auto cpos = editor.GetCursorPosition();
        ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar /*| ImGuiWindowFlags_NoMove*/);  // change for docking
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))            { editor.SetText(""); fileName = "Untitled"; }  //?? handle file name and extension seperately?
                if (ImGui::MenuItem("Open", "Ctrl+O")) { ImGuiFileDialog::Instance()->OpenModal("ChooseFileKey", "Choose File", fileTypeFilter, ".", ""); }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    if (fileName == "Untitled") ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", fileTypeFilter, ".", "");
                    else
                    {
                        std::ofstream outfile;
                        outfile.open(fileName);
                        outfile << std::noskipws;
                        // pop up for failure? if (!outfile) { //stuff }
                        outfile << editor.GetText();
                        outfile.close();
                    }
                }
                if (ImGui::MenuItem("Save As..")) { ImGuiFileDialog::Instance()->OpenModal("SaveAsKey", "Save File As", fileTypeFilter, ".", ""); }
                
                if (ImGui::MenuItem("Quit", "Alt-F4")) window.close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor.IsReadOnly();
                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))                                          editor.SetReadOnly(ro);
                ImGui::Separator();    

                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))               editor.Undo();
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))                      editor.Redo();
                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))                        editor.Copy();
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))                  editor.Cut();
                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))                  editor.Delete();
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr)) editor.Paste();
                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr)) 
                    editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Dark palette"))       editor.SetPalette(TextEditor::GetDarkPalette());
                if (ImGui::MenuItem("Light palette"))      editor.SetPalette(TextEditor::GetLightPalette());
                if (ImGui::MenuItem("Retro blue palette")) editor.SetPalette(TextEditor::GetRetroBluePalette());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), fileName.c_str());

        ImGui::Separator();
        ImGui::Spacing();

        editor.Render("TextEditor");
        if (ImGui::Button("Tokenize"))  // Why is button barely in view??..
        {
            // RUN LEXER!
            // Pass output to another ImGuiWindow to display a table of tokens and their info
        }
        ImGui::End();

        if (ImGuiFileDialog::Instance()->Display("ChooseFileKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::ifstream infile;
                fileName = ImGuiFileDialog::Instance()->GetFilePathName(); // GetSelectionDifferences?
                infile.open(fileName);
                // pop up for failure? if (!infile) { //stuff }
                std::vector<std::string> lines;
                std::string line;
                while (std::getline(infile, line)) lines.push_back(line);
                editor.SetTextLines(lines);

                infile.close();
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveAsKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::ofstream outfile;
                std::string fpName = ImGuiFileDialog::Instance()->GetFilePathName();
                outfile.open(fpName);
                outfile << std::noskipws;
                // pop up for failure? if (!outfile) { //stuff }
                outfile << editor.GetText();
                fileName = fpName;
                outfile.close();

            }
            ImGuiFileDialog::Instance()->Close();
        }

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
}

/*  TODO:
*       Overwriting confirmation for SaveAs
*       Popup dialog for file errors
*       Shortcut implementation
*       Lexer Integration
*       Cleanup/Separation of display etc
*       Docking
*       Merge all compiler branches, create new on (dissertation)
*           Master becomes an ImGui frontend
*/