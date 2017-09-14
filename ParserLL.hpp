
#ifndef PARSERLL
#define PARSERLL

#include "astring_view.hpp"

namespace xts {

   class ParserLL
   {
   public:
      ParserLL(astd::string_view buffer)
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

      bool consume(char c)
      {
         if (peek(c))
         {
            _index++;
            return true;
         }
         return false;
      }

      bool peek(astd::string_view s)
      {
         unsigned int i = 0;
         while (i < s.size() && _index + i < _buffer.size() && _buffer[_index + i] == s[i])
         {
            i++;
         }
         return (i >= s.size());
      }

      bool consume(astd::string_view s)
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
         return consume("\n") || consume("\r\n") || consume("\r");
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
         while (consume('\r') || consume('\n') || consume(' ') || consume('\t'));
         return true;
      }

      bool tryignoreBlanks()
      {
         if (consume('\r') || consume('\n') || consume(' ') || consume('\t'))
         {
            while (consume('\r') || consume('\n') || consume(' ') || consume('\t'));
            return true;
         }
         return false;
      }

      bool ignoreUntil(astd::string_view p)
      {
         while (!peek(p))
         {
            _index++;
         }
         return consume(p);
      }

	  bool parseintoSize(std::size_t num, astd::string_view& into)
	  {
		  bool result = _buffer.size() - _index >= num;
		  if (result)
		  {
			   into = _buffer.substr(_index, num);
			   _index += num;
		  }
		  return result;
	  }

      bool parseintoUntil(astd::string_view limiter, astd::string_view& into)
      {
         int _index_tmp = _index;
         while (!peek(limiter))
         {
            _index++;
         }
         into = _buffer.substr(_index_tmp, _index - _index_tmp);
         if (consume(limiter))
         {
            return true;
         }
         _index = _index_tmp;
         return false;
      }

	  template <typename T>
	  bool fill(T& value)
	  {
		  bool result = sizeof(T) < (_buffer.size() - _index);
		  if (result)
		  {
			  value = *(reinterpret_cast<const T*>(_buffer.data() + _index));
		  }
		  return result;
	  }

	  template <typename T>
	  bool fconsume(T& value)
	  {
		  bool result = fill(value);
		  if (result)
			  _index += sizeof(T);
		  return result;
	  }

   protected:
      std::size_t _index;
	  astd::string_view _buffer;
   };

}

#endif /* !PARSERLL */