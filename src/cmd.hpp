#ifndef CMD_HPP
#define CMD_HPP

#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <cctype>

namespace ftp {

enum CMD { 
    ABOR, /* ABOR : terminate current operation                             */
    CWD,  /* CWD <directory>                                                */
    DELE, /* DELE <filename> : delete                                       */
    HELP, /* HELP <command>                                                 */
    LIST, /* LIST <name> : return file or directory info                    */

    MKD,  /* MKD <directory                                                 */ 
    MODE, /* MODE <mode> : transfer mode                                    */

    NLST, /* NLST <directory> : directory info                              */
    NOOP, /* : no operation                                                 */

    PORT, /* PORT <address> get ip and port of server                       */
    PASV, /* PASV: passive                                                  */
    PASS, /* PASS <password>                                                */
    PWD,  /* PWD : current dir                                              */

    RMD,  /* RMD <directory>                                                */
    RETR, /* RETR <filename> : download                                     */

    STOR, /* STOR <filename> : upload                                       */
    STRU, /* STRU <structure> : data structure (stream, block, compressed)  */
    SYST, /* : OS info                                                      */

    TYPE, /* TYPE <type> : A=ASCII, E=EBCDIC, I=BINARY                      */
    QUIT, /* : sign out                                                     */
    USER, /* USER <username>                                                */
    
    BAD   /* : unknown cmd                                                    */
};

CMD ResolveCMD(const std::string& cmd) {
    std::string upper_cmd;
    std::transform(cmd.begin(), cmd.end(), 
                    std::back_inserter(upper_cmd), toupper);
    static std::unordered_map<std::string, CMD> cmd_table = {
        { "ABOR", ABOR },
        { "CWD",  CWD  },
        { "DELE", DELE },
        { "HELP", HELP },
        { "LIST", LIST },
        { "MKD",  MKD  },
        { "MODE", MODE },
        { "NLST", NLST },
        { "NOOP", NOOP },
        { "PORT", PORT },
        { "PASV", PASV },
        { "PASS", PASS },
        { "PWD",  PWD  },
        { "RMD",  RMD  },
        { "RETR", RETR },
        { "STOR", STOR },
        { "STRU", STRU },
        { "SYST", SYST },
        { "TYPE", TYPE },
        { "QUIT", QUIT },
        { "USER", USER },
    };
    if (cmd_table.find(upper_cmd) != cmd_table.end())
        return cmd_table[upper_cmd];
    return BAD;
}

std::string Response(int code) {
    std::unordered_map<int, std::string> response_table = {
        { 120, "Service ready in nnn minutes."                        },
        { 125, "Data connection already open: transfer starting"      },
        { 150, "File status okay; about to open data conenction."     },
        { 200, "Command okay."                                        },
        { 211, "System status, or system help reply."                 },
        { 212, "Directory status."                                    },
        { 213, "File status."                                         },
        { 214, "Help message."                                        },
        { 215, "Name system type."                                    },
        { 220, "Service ready for new user."                          },
        { 221, "Service closing control connection."                  },
        { 225, "Data connection open: no transfer in progress."       },
        { 226, "Closing data connections."                            },
        { 227, "Entering Passive Mode(h1,h2,h3,h4,p1,p2)."            },
        { 230, "User logged in, preceed."                             },
        { 250, "Requestd file action okay, completed."                },
        { 331, "User name okay, need password."                       },
        { 332, "Need account for login."                              },
        { 350, "Requestd file action pending further infromation."    },
        { 421, "Service not available, closing control connection."   },
        { 425, "Can't open data connection."                          },
        { 426, "Connection closed, transfer aborted."                 },
        { 450, "Requested file action not taked."                     },
        { 451, "Requested action aborted, local error in processsing."},
        { 452, "Requested action not taken."                          },
        { 500, "Syntax error, command unrecognized."                  },
        { 501, "Syntax error in parameters or arguments."             },
        { 502, "Command not implemented."                             },
        { 503, "Bad sequence of commands."                            },
        { 504, "Command not implemented for that parameter."          },
        { 530, "Not logged in."                                      },
        { 550, "Requested action not taken."                          },
        { 551, "Requested action aborted: page type unknown."         },
        { 552, "Requested action aborted: Exceeded storage allcation."},
        { 553, "Requested action not taken. File name not allowed."   }
    };
    if (response_table.find(code) != response_table.end())
        return response_table[code];
    assert(false);
}

}// end of namespace ftp
#endif
