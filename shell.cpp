// #include <iostream>
// #include <string>
// #include "shell.h"
// #include "staging.h"
// #include "commit.h"

// using namespace std;

// void startShell(StagingArea& staging , CommitNode* head , string author , string branch)
// {
//     string command;

//     while (true)
//     {
//         cout << "minivcs> " << " ";
//         getline(cin, command);
//         if (command == "exit")
//         {
//             cout << "Exiting MiniVCS...";
//         }

//         else if (command == "help")
//         {
//             cout << "\n";
//             cout << "========================================\n";
//             cout << "          MiniVCS - Help Menu\n";
//             cout << "========================================\n";

//             cout << "\nUsage:\n";
//             cout << "  minivcs> <command> [arguments]\n";

//             cout << "\nAvailable Commands:\n";
//             cout << "  help                     Show this help menu\n";
//             cout << "  add <filename>           Stage a file for commit\n";
//             cout << "  commit -m <message>      Commit staged files with a message\n";
//             cout << "  status                   Show the current staging status\n";
//             cout << "  undo                     Undo the last operation\n";
//             cout << "  login                    Log in to MiniVCS\n";
//             cout << "  logout                   Log out of the current account\n";
//             cout << "  exit                     Exit MiniVCS\n";

//             cout << "\nExamples:\n";
//             cout << "  minivcs> add notes.txt\n";
//             cout << "  minivcs> commit -m \"Added notes file\"\n";
//             cout << "  minivcs> status\n";
//             cout << "  minivcs> undo\n";

//             cout << "\nNotes:\n";
//             cout << "  - Files must be staged before committing.\n";
//             cout << "  - A commit message is required for every commit.\n";
//             cout << "  - Use 'help' to view commands at any time.\n";

//             cout << "========================================\n\n";
//         }
//         else if (command.find("commit -m") == 0){
//             string msg = command.substr(10); // extracting command message from the commit 
//             if(msg.size() == 0) 
//             {   cout << "Please provide a commit message... \n" ;
//                 continue;
//             }
//             cout << "Message is : " << msg ; // testing
//             if(staging.isEmpty()){
//                 cout << "Add a file first..." << endl;
//             }
//             else{
//                 addCommit(head , author , msg , branch);
//             }
//         }

//         else if(command.find("add") == 0){
//             string file = command.substr(4);
            
//         }
//     }
// }