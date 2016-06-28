
#ifndef PARSERLL
#define PARSERLL

#include <string>

namespace xts {

   class ParserLL
   {
   public:
      ParserLL(std::string buffer)
         : _index(0), _buffer(buffer)
      {}

      inline bool eof()
      {
         return _index >= _buffer.size();
      }

      bool peek(char c)
      {
         return (!eof() && _buffer[_index] == c);
      }

      bool parse(char c)
      {
         if (peek(c))
         {
            _index++;
            return true;
         }
         return false;
      }

      bool peek(std::string s)
      {
         unsigned int i = 0;
         while (i < s.size() && _index + i < _buffer.size() && _buffer[_index + i] == s[i])
         {
            i++;
         }
         return (i >= s.size());
      }

      bool parse(std::string s)
      {
         if (peek(s))
         {
            _index += s.size();
            return true;
         }
         return false;
      }

      bool peek_endl()
      {
         return peek("\n") || peek("\r\n") || peek("\r");
      }

      bool parse_endl()
      {
         return parse("\n") || parse("\r\n") || parse("\r");
      }

      bool peek_between(char start, char end)
      {
         return (!eof() && _buffer[_index] >= start && _buffer[_index] <= end);
      }

      bool parse_between(char start, char end)
      {
         if (peek_between(start, end))
         {
            _index++;
            return true;
         }
         return false;
      }

      bool ignoreBlanks()
      {
         while (parse('\r') || parse('\n') || parse(' ') || parse('\t'));
         return true;
      }

      bool tryignoreBlanks()
      {
         if (parse('\r') || parse('\n') || parse(' ') || parse('\t'))
         {
            while (parse('\r') || parse('\n') || parse(' ') || parse('\t'));
            return true;
         }
         return false;

      }

      bool ignoreUntil(std::string p)
      {
         while (!peek(p))
         {
            _index++;
         }
         return parse(p);
      }

      bool parseintoUntil(std::string limiter, std::string& into)
      {
         int _index_tmp = _index;
         while (!peek(limiter))
         {
            _index++;
         }
         into += _buffer.substr(_index_tmp, _index - _index_tmp);
         if (parse(limiter))
         {
            return true;
         }
         _index = _index_tmp;
         return false;
      }

   protected:
      unsigned int _index;
      std::string _buffer;
   };

}

#endif /* !PARSERLL */