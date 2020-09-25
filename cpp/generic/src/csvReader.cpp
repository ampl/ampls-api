#include "csvReader.h"

namespace ampls {
namespace impl {

namespace CSVState {
enum Value {
  NameField,
  UnquotedField,
  QuotedField,
  QuotedQuote
};
}

std::vector<std::string> readAMPLNameRow(const std::string& row) {
  CSVState::Value state = CSVState::NameField;

  std::vector<std::string> fields(1, "");
  char CurrentQuote;

  size_t i = 0; // index of the current field
  for (size_t p = 0; p < row.size(); p++) {
    const char c = row[p];
    switch (state)
    {
    case CSVState::NameField:
      switch (c)
      {
      case '[': // end of name
        fields.push_back("");
        i++;
        state = CSVState::UnquotedField;
        break;
      default:
        fields[i].push_back(c);
        break;
      }
      break;
    case CSVState::UnquotedField:
      switch (c)
      {
      case ',': // end of field
      case ']': // end of indexing expression
        fields.push_back("");
        i++;
        break;
      case '"':
      case '\'':
        state = CSVState::QuotedField;
        CurrentQuote = c;
        break;
      default:
        fields[i].push_back(c);
        break;
      }
      break;
    case CSVState::QuotedField:
    {
      if (c == CurrentQuote)
      {
        state = CSVState::UnquotedField;
        break;
      }
    default:
      fields[i].push_back(c);
      break;
    }
    break;
    }
  }
  fields.pop_back();
  return fields;
}

std::vector<std::vector<std::string> > readCSV(std::istream& in) {
  std::vector<std::vector<std::string> > table;
  std::string row;
  while (!in.eof()) {
    std::getline(in, row);
    if (in.bad() || in.fail()) {
      break;
    }
    std::vector<std::string> fields = readAMPLNameRow(row);
    table.push_back(fields);
  }
  return table;
}

bool startsWith(std::string mainStr, std::string toMatch)
{
  return mainStr.find(toMatch) == 0;
}

std::map<std::string, int> createMap(std::istream& in, const char* beginWith) {
  std::map<std::string, int> map;
  int i = 0;
  std::string row;
  while (!in.eof()) {
    std::getline(in, row);
    if (in.bad() || in.fail()) {
      break;
    }
    if ((beginWith == NULL) || (startsWith(row, beginWith)))
      map[row] = i++;
  }
  return map;
}

std::map<int, std::string> createMapInverse(std::istream& in) {
  std::map<int, std::string> map;
  int i = 0;
  std::string row;
  while (!in.eof()) {
    std::getline(in, row);
    if (in.bad() || in.fail()) {
      break;
    }
    map[i++] = row;
  }
  return map;
}
} // namespace impl
} // namespace ampls
