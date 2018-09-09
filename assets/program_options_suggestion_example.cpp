#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>

#include <exception>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;

unsigned int
levenshtein_distance(const std::string &s1,
                     const std::string &s2) {
  const std::size_t len1 = s1.size(), len2 = s2.size();
  std::vector<unsigned int> col(len2 + 1),
      prevCol(len2 + 1);

  for (unsigned int i = 0; i < prevCol.size(); i++)
    prevCol[i] = i;
  for (unsigned int i = 0; i < len1; i++) {
    col[0] = i + 1;
    for (unsigned int j = 0; j < len2; j++)
      col[j + 1] = std::min(
          {prevCol[1 + j] + 1, col[j] + 1,
           prevCol[j] + (s1[i] == s2[j] ? 0 : 1)});
    col.swap(prevCol);
  }
  return prevCol[len2];
}

std::vector<std::string> findSimilarOptions(
    const std::string &name,
    const std::vector<std::string> &options) {
  constexpr size_t MAX_SUGGESTIONS = 7;

  std::vector<std::string> subStringOptions;
  std::vector<std::string> levenshteinOptions;

  // Collect substring options
  {
    // Strip dashes
    auto nameStrip = name;
    nameStrip.erase(std::remove(nameStrip.begin(),
                                nameStrip.end(), '-'),
                    nameStrip.end());

    // Suggest options containing the
    // substring
    for (const auto &optionName : options) {
      if (subStringOptions.size() >= MAX_SUGGESTIONS)
        break;

      if (optionName.find(nameStrip) !=
          std::string::npos)
        subStringOptions.push_back(optionName);
    }
  }

  // Collect best levenshtein distance options
  {
    size_t bestDist = UINT_MAX;

    for (const auto &optionName : options) {
      if (levenshteinOptions.size() >= MAX_SUGGESTIONS)
        break;

      const auto dist =
          levenshtein_distance(name, optionName);
      if (dist < bestDist) {
        levenshteinOptions.clear();
        bestDist = dist;
      }

      if (dist == bestDist &&
          std::find(subStringOptions.begin(),
                    subStringOptions.end(),
                    optionName) ==
              subStringOptions.end())
        levenshteinOptions.push_back(optionName);
    }
  }

  // Merge suggestions giving precedence to
  // substrings
  auto output = subStringOptions;
  for (const auto &option : levenshteinOptions)
    output.push_back(option);
  output.resize(
      std::min(output.size(), MAX_SUGGESTIONS));

  return output;
}

int main(int argc, char **argv) {
  po::options_description desc("Example");
  desc.add_options()
      //
      ("foo", po::value<int>(), "Foo")
      //
      ("bar", po::value<int>(), "bar")
      //
      ("baz", po::value<int>(), "baz");

  po::variables_map vm;

  try {
    po::parsed_options parsedOptions =
        po::command_line_parser(argc, argv)
            .options(desc)
            .allow_unregistered()
            .style(
                po::command_line_style::unix_style &
                ~po::command_line_style::allow_short &
                ~po::command_line_style::allow_guessing)
            .run();

    const auto unrecognizedOptions =
        po::collect_unrecognized(parsedOptions.options,
                                 po::exclude_positional);

    std::vector<std::string> availableOptions;
    for (auto option : desc.options())
      availableOptions.push_back(option->format_name());

    if (!unrecognizedOptions.empty()) {
      const auto &unrecognized =
          unrecognizedOptions.front();

      const auto suggestions = findSimilarOptions(
          unrecognized, availableOptions);

      std::string errorMessage =
          "Unrecognized option '" + unrecognized +
          "'.\n\nMost similar options are:";

      for (const auto &suggestion : suggestions)
        errorMessage += "\n\t" + suggestion;

      throw po::error(errorMessage);
    }

    po::store(parsedOptions, vm);
    po::notify(vm);
  } catch (po::error &e) {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  return 0;
}