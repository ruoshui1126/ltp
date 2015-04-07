// Defines the entry point for the Web Service application.
//

#include <sys/wait.h>
#include <unistd.h>       /* For pause() */
#include <stdlib.h>
#include <signal.h>

#include <iostream>

#include "mongoose.h"

#include "Xml4nlp.h"
#include "Ltp.h"

#include "codecs.hpp"
#include "logging.hpp"

#if !defined(LISTENING_PORT)
#define LISTENING_PORT	"12345"
#endif /* !LISTENING_PORT */

#define POST_LEN 1024

using namespace std;
using namespace ltp::strutils::codecs;

static LTP * engine = NULL;

static int exit_flag;

static int Service(struct mg_connection *conn);

static void ErrorResponse(struct mg_connection * conn,
                          enum ErrorCodes code);

static void signal_handler(int sig_num) {
  exit_flag = sig_num;
}

int main(int argc, char *argv[]) {
  engine = new LTP;

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  struct mg_context *ctx;
  const char *options[] = {"listening_ports", LISTENING_PORT,
    "num_threads", "1", NULL};
  struct mg_callbacks callbacks;

  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = Service;

  if ((ctx = mg_start(&callbacks, NULL, options)) == NULL) {
    ERROR_LOG("Cannot initialize Mongoose context");
    exit(EXIT_FAILURE);
  }

  // getchar();
  while (exit_flag == 0) {
    sleep(100000);
  }
  mg_stop(ctx);

  return 0;
}

/*
 * Use to 
 *
 *
 */
static void ErrorResponse(struct mg_connection * conn,
                          enum ErrorCodes code) {
  switch (code) {
    case kEmptyStringError: 
      {
        std::string response = "HTTP/1.1 400 EMPTY SENTENCE\r\n\r\n";
        mg_printf(conn, "%s", response.c_str());
        break;
      }
    case kEncodingError: 
      {
        // Input sentence is not clear
        std::string response = "HTTP/1.1 400 ENCODING NOT IN UTF8\r\n\r\n";
        mg_printf(conn, "%s", response.c_str());
        break;
      }
    case kXmlParseError : 
      {
        // Failed the xml validation check
        std::string response = "HTTP/1.1 400 BAD XML FORMAT\r\n\r\n";
        response += "Failed to load custom xml";
        mg_printf(conn, "%s", response.c_str());
        break;
      }
    case kSentenceTooLongError:
      {
        std::string response = "HTTP/1.1 400 SENTENCE TOO LONG\r\n\r\n";
        response += "Input sentence exceed 300 characters or 70 words";
        mg_printf(conn, "%s", response.c_str());
        break;
      }
    case kSplitSentenceError:
    case kWordsegError:
    case kPostagError:
    case kParserError:
    case kNERError:
    case kSRLError:
      {
        std::string response = "HTTP/1.1 400 ANALYSIS FAILED\r\n\r\n";
        mg_printf(conn, "%s", response.c_str());
        break;
      }
    default:
      {
        ERROR_LOG("Non-sence error catch");
        break;
      }
  }
}

static int Service(struct mg_connection *conn) {
  char *sentence;
  char type[10];
  char xml[10];
  char cus[10];
  char m_path[200];
  char l_path[200];

  string str_post_data;
  string str_type;
  string str_xml;

  const struct mg_request_info *ri = mg_get_request_info(conn);

  if (!strcmp(ri->uri, "/ltp")) {
    int len;
    char buffer[POST_LEN];

    while((len = mg_read(conn, buffer, sizeof(buffer) - 1)) > 0){
      buffer[len] = 0;
      str_post_data += buffer;
    }

    TRACE_LOG("CDATA: %s", str_post_data.c_str());
    TRACE_LOG("CDATA length: %d", str_post_data.size());

    sentence = new char[str_post_data.size() + 1];

    mg_get_var(str_post_data.c_str(),
               str_post_data.size(),
               "s",
               sentence,
               str_post_data.size());

    mg_get_var(str_post_data.c_str(),
               str_post_data.size(),
               "t",
               type,
               sizeof(type) - 1);

    mg_get_var(str_post_data.c_str(),
               str_post_data.size(),
               "x",
               xml,
               sizeof(xml) - 1);

    mg_get_var(str_post_data.c_str(),
               str_post_data.size(),
               "c",
               cus,
               sizeof(cus) - 1);

    string strSentence = sentence;

    // validation check
    if (strlen(sentence) == 0) {
      WARNING_LOG("Input sentence is empty");
      ErrorResponse(conn, kEmptyStringError);
      return 0;
    }

    if (!isclear(strSentence)) {
      WARNING_LOG("Failed string validation check");
      ErrorResponse(conn, kEncodingError);
      return 0;
    }

    if(strlen(type) == 0) {
      str_type = "";
    } else {
      str_type = type;
    }

    if(strlen(xml) == 0) {
      str_xml = "";
    } else {
      str_xml = xml;
    }

    delete []sentence;

    TRACE_LOG("Input sentence is: %s", strSentence.c_str());

    //Get a XML4NLP instance here.
    XML4NLP xml4nlp;

    if (str_xml == "y") {
      if (-1 == xml4nlp.LoadXMLFromString(strSentence)) {
        // failed the xml validation check
        return 0;
      }

      // move sentence validation check into each module
    } else {
      xml4nlp.CreateDOMFromString(strSentence);
    }

    TRACE_LOG("XML Creation is done.");

    if (strcmp(cus,"y")==0) {
    TRACE_LOG("cus = y");
      mg_get_var(str_post_data.c_str(),
                 str_post_data.size(),
                 "m",
                  m_path,
                 sizeof(m_path) - 1);

      mg_get_var(str_post_data.c_str(),
                 str_post_data.size(),
                 "l",
                  l_path,
                 sizeof(l_path) - 1);
      if (!(strlen(m_path)==0&&strlen(l_path)==0)) {
        TRACE_LOG(m_path);
        TRACE_LOG(l_path);
        xml4nlp.SetNote(NOTE_CUS);
        xml4nlp.SetNoteValue(NOTE_MDL, m_path);
        xml4nlp.SetNoteValue(NOTE_LEX, l_path);
      }
    }

    if (str_type == "ws") {
      engine->wordseg(xml4nlp);
    } else if (str_type == "pos") {
      engine->postag(xml4nlp);
    } else if (str_type == "ner") {
      engine->ner(xml4nlp);
    } else if (str_type == "dp") {
      engine->parser(xml4nlp);
    } else if (str_type == "srl") {
      engine->srl(xml4nlp);
    } else {
      engine->srl(xml4nlp);
    }

    TRACE_LOG("Analysis is done.");

    string strResult;
    xml4nlp.SaveDOM(strResult);

    strResult = "HTTP/1.1 200 OK\r\n\r\n" + strResult;
    mg_printf(conn, "%s", strResult.c_str());

  }
  return 1;
}

