
class Json {

  //returns a Wren Map or List from a json string 
  static parse(string) { parse("json", string) }
  static parse(source_id, source_string) {
    if(source_string == null) Fiber.abort("Expected string value for Json.parse")
    var parser = JsonParser.new(source_id, source_string)
    var root = parser.root
    parser = null
    return root
  }

    //default is two spaces
  static stringify(value) { stringify(value, "  ") }

    //with custom whitespace
  static stringify(value, whitespace) {
    var string = ""
    JsonStringify.stringify(value, whitespace) {|add|
      string = string + add
    }
    return string
  }

    //with custom callback
  static stringify(value, whitespace, callback) {
    JsonStringify.stringify(value, whitespace, callback)
  }

} //Json

//Handles actual stringification,
//it calls out to the provided callback to handle "writing"
//which means you can e.g write to a file directly, or into a buffer, 
//which is often significantly faster than concatenating strings
class JsonStringify {

  static stringify(value, whitespace, out) {

    if(!(value is Map || value is List)) {
      Fiber.abort("Json stringify requires a Map or a List as input")
    }

    if(value is Map) {
      stringify_map(value, whitespace, 0, out)
    } else {
      stringify_value(value, whitespace, 0, out)
    }

  } //stringify

  static stringify_map(map, whitespace, depth, out) {

    if(!(map is Map)) Fiber.abort("Expected a Map, received %(map.type) as %(map)")
    if(map.count == 0) return out.call("{}")

    out.call("{\n")

      var index = 0
      var count = map.count
      for(key in map.keys) {

        out.call(whitespace * (depth + 1))
        out.call("\"%(key)\" : ")

        var value = map[key]
        stringify_value(value, whitespace, depth + 1, out)

        var last = index == count-1
        out.call(last ? "\n" : ",\n")

        index = index + 1

      } //each key

    out.call(whitespace * depth)
    out.call("}")

  } //stringify_map

  static stringify_primitive(value, out) {

    if(value is String) {
      var string = "%(value)"
        //basic json escaping. todo: this isn't extensive
        string = string.replace("\\", "\\\\") // double backslash must be first
        string = string.replace("\"", "\\\"") // then replace single 
        string = string.replace("\n", "\\n")  
      out.call("\"%(string)\"")
    } else if(value is Num || value is Null || value is Bool) {
      out.call("%(value)")
    } else {
      Fiber.abort("Can't stringify type %(value.type)!")
    }

  } //stringify_primitive

  static stringify_list(list, whitespace, depth, out) {

    if(!(list is List)) Fiber.abort("Expected a List, received %(list.type) as %(list)")

      //clearer output if empty, simpler code
    if(list.count == 0) return out.call("[]")

    out.call("[\n")

      var index = 0
      var count = list.count
      for(item in list) {
        out.call(whitespace * (depth + 1))
        stringify_value(item, whitespace, depth + 1, out)
        var last = index == count - 1
        out.call(last ? "\n" : ",\n")
        index = index + 1
      } //each item

    out.call(whitespace * depth)
    out.call("]")

  } //stringify_list

  static stringify_value(value, whitespace, depth, out) {
    if(value is Map) {
      stringify_map(value, whitespace, depth, out)
    } else if(value is List) {
      stringify_list(value, whitespace, depth, out)
    } else {
      stringify_primitive(value, out)
    }
  } //stringify_value

} //JsonStringify


//There's some weird choices here from early on using wren but /shrug :)
class JsonParser {

  root { _root }

  construct new(source_id, source) {

    _source_id = source_id
    _root = null

    _EOF            = -1
    _ERR            = -2
    _NEW_LINE       = 10 //\n LF
    _OPEN_BRACE     = 123
    _CLOSE_BRACE    = 125
    _OPEN_BRACKET   = 91
    _CLOSE_BRACKET  = 93
    _OPEN_STRING    = 34
    _CLOSE_STRING   = 34
    _COLON          = 58
    _COMMA          = 44
    _EQUAL          = 61
    _TAB            = 9
    _SPACE          = 32
    _COMMENT        = 47
    _ESCAPED        = 92

      //:todo: we could use a token and do it inline but for now
    source = source.replace("\r\n", "\n")

    _cur = -1
    _line = 0
    _col = 0
    _end = source.count
    _points = source.codePoints
    _in_comment = 0

    var pk = peek()
    if(pk == _OPEN_BRACKET) {
      _root = parse_list()
    } else if(pk == _OPEN_BRACE) {
      _root = parse_map()
    } else {
      Fiber.abort("expected a Map or List at the root")
    }

  } //new

  unexpected(point) {
    var err = "end of file"
    if(point != -1) err = String.fromCodePoint(point)
    Fiber.abort("unexpected `%(err)` at line %(_line):%(_col) (%(point)) in file `%(_source_id)`")
  } //unexpected

  is_eof(point) { point == _EOF }
  is_whitespace(point) { point == _SPACE || point == _TAB  || point == _NEW_LINE }
  is_token(point) {
    return  point == _COMMA ||
            point == _COLON ||
            point == _CLOSE_BRACE ||
            point == _OPEN_BRACE ||
            point == _CLOSE_BRACKET ||
            point == _OPEN_BRACKET
  } //is_token

  next() {
    _cur = skips(true) + 1
    if(_cur >= _end) return _EOF
    return _points[_cur]
  } //next

  peek() { peek(1) }
  peek(n) {
    var idx = skips(false) + n
    if(idx >= _end) return _EOF
    return _points[idx]
  } //peek

  peeks() { peeks(1) }
  peeks(n) {
    if(_cur+n >= _end) return _EOF
    return _points[_cur+n]
  } //peeks

  step(consume) {

    var cur = _cur + 1
    if(cur >= _end) {
      if(consume) _cur = _end
      return _EOF
    }

    if(consume) {
      _cur = cur
      if(_points[cur] == _NEW_LINE) {
        _col = 0
        _line = _line + 1
      } else {
        _col = _col + 1
      }
    }

    return _points[_cur]

  } //step

  skips(consume) {

    var cur = _cur
    while(true) {

      cur = cur + 1
      if(cur >= _end) {
        if(consume) _cur = _end
        return _end
      }

      var tok = _points[cur]

        //if not in comment state, increment state count
      if(_in_comment <= 1 && tok == _COMMENT) {
        //as long as the next token is actually a comment,
        //and the next token isn't outside the bounds of the string
        var nxt = (cur+1).min(_end-1)
        var nxt_tok = _points[nxt]
        if(nxt > cur && nxt_tok == _COMMENT) {
          _in_comment = _in_comment + 1
        }
      }

        //end of line is a comment state reset, must be before break
      if(tok == _NEW_LINE) _in_comment = 0

        //if not inside a comment, normal tokens apply
      if(_in_comment == 0 && !is_whitespace(tok)) {
        break
      }

      if(consume) {
        _cur = cur
        if(tok == _NEW_LINE) {
          _col = 0
          _line = _line + 1
        } else {
          _col = _col + 1
        }
      }

    } //while

    return cur - 1

  } //skips

  parse_key() {

    var key = ""
    var quoted = peek() == _OPEN_STRING
    if(quoted) next()

    var first = next()
    if(!quoted && (first == _COLON || first == _EQUAL)) unexpected(first)
    if(is_token(first)) unexpected(first)
    if(is_eof(first)) unexpected(_EOF)

    //:todo: escapes in keys are not handled

    key = key + String.fromCodePoint(first)
    while(true) {
      var pk = peek()
      if(quoted) {
        if(pk == _CLOSE_STRING) break
      } else {
        if(pk == _COLON || pk == _EQUAL || pk == _CLOSE_STRING) break
      }
      var point = step(true)
      if(is_eof(point)) unexpected(_EOF)
      var upcoming = peek()
      var is_ending = (upcoming == _COLON || upcoming == _EQUAL)
      if(!is_ending) {
        key = key + String.fromCodePoint(point)
      } else if(!is_whitespace(point)) {
        key = key + String.fromCodePoint(point)
      }
    }

    if(!quoted) {
      if(peeks() == _CLOSE_STRING) Fiber.abort("closing quote without open quote for key at line %(_line):%(_col) in file `%(_source_id)`")
    } else {
      var was = next() //disard the close quote
      if(was != _CLOSE_STRING) Fiber.abort("unclosed quotes for key at line %(_line):%(_col), a \" is missing here, in file `%(_source_id)` %(String.fromCodePoint(was))") //\"
    }

    next() //discard colon/equal
    return key

  } //parse_key

  parse_primitive() {

    var prim = ""

    //consume whitespace
    skips(true)

    while(true) {

      var pk = peeks()
      if(is_token(pk)) break
      if(is_whitespace(pk)) break

      var point = next()

      if(is_eof(point)) unexpected(_EOF)

      prim = prim + String.fromCodePoint(point)

    } //while

    return prim

  } //parse_primitive

  read_raw_string() {

    var string = ""
    var eof = false
    var cur = _cur + 1
    while(true) {
      if(cur >= _end) {
        eof = true
        break
      }
      var point = _points[cur]
      var point1 = cur+1 < _end ? _points[cur+1] : -1
      var point2 = cur+2 < _end ? _points[cur+2] : -1
      if(point1 == -1 || point2 == -1) break //:todo: give an error when past the end

      if(point == _CLOSE_STRING && point1 == _CLOSE_STRING && point2 == _CLOSE_STRING) {
        cur = cur + 2
        break
      }

      var is_invalid = point == -1
      if(!is_invalid) {
        string = string + String.fromCodePoint(point)
      }
      cur = cur + 1
    }

    _cur = cur //this consumes the close quote

    return string

  } //read_raw_string

  read_string() {

    var string = ""
    var eof = false
    var cur = _cur + 1
    var prev = _OPEN_STRING
    while(true) {
      if(cur >= _end) {
        eof = true
        break
      }
      var point = _points[cur]
      var is_invalid = point == -1
      if(point == _CLOSE_STRING && prev != _ESCAPED) break
      var escaped = point == _ESCAPED && _points[cur+1] == _CLOSE_STRING
      if(!escaped && !is_invalid) {
        string = string + String.fromCodePoint(point)
      }
      if(!is_invalid) prev = point
      cur = cur + 1
    }

    _cur = cur //this consumes the close quote

    return string
  } //read_string

  parse_string() {
    
    next() //consume open quote

    if(peeks() == _OPEN_STRING && peeks(2) == _OPEN_STRING) {
      next()
      next()
      return read_raw_string()
    }

    return read_string()

  } //parse_string

  parse_value() {

    var pk = peek()

    if(pk == _OPEN_BRACE)   return parse_map()
    if(pk == _OPEN_BRACKET) return parse_list()
    if(pk == _OPEN_STRING)  return parse_string()

    var prim = parse_primitive()

    if(prim == "false") return false
    if(prim == "true") return true
    if(prim == "null") return null

    return Num.fromString(prim)

  } //parse_value

  parse_list() {

    next() //consume the open bracket

    var list = []

    while(true) {

      if(peek() == _CLOSE_BRACKET) break

      list.add(parse_value())

      var pk = peek()
      if(pk == _CLOSE_BRACKET) break
      if(pk == _COMMA) next() //consume comma, keep going

    } //while

    next() //consume close bracket

    return list

  } //parse_list

  parse_map() {

    next() //consume the open brace

    var map = parse_map_value()

    next() //consume close brace

    return map

  } //parse_map

  parse_map_value() {

    var map = {}

    while(true) {
      if(peek() == _EOF) break
      if(peek() == _CLOSE_BRACE) break

      var key = parse_key()
      var value = parse_value()
      map[key] = value

      var pk = peek()
      if(peek() == _CLOSE_BRACE) break
      if(pk == _COMMA) next() //consume comma, keep going
    }

    return map

  } //parse_map

} //JsonParser
