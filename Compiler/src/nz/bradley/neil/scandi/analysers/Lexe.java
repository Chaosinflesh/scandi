package nz.bradley.neil.scandi.analysers;

import java.util.ArrayList;
import java.util.List;

public class Lexe {

    /**
     *
     * @param data  A map containing all the lines for each file, with
     *              comments and empty lines removed.
     *
     * @return  The file contents, tokenized.
     */
    public static List<String> analyse(String relativeDotPath, List<String> data, List<String> warnings) {
        var name = relativeDotPath.contains(".")
                ? relativeDotPath.substring(relativeDotPath.lastIndexOf("."))
                : relativeDotPath;
        var lexemes = new ArrayList<String>();
        int count = 0;

        for (var line: data) {
            count++;
            lexemes.add(";" + getDepth(line));
            lexemes.addAll(splitWithStrings(line.stripLeading(), relativeDotPath, count, warnings));
        }

        return lexemes;
    }


    private static int getDepth(String line) {
        // Files have depth of 1 - globals are at 0.
        return 1 + line.length() - line.stripLeading().length();
    }


    private static List<String> splitWithStrings(String line, String dotPath, int lineNo, List<String> warnings) {
        String symbols = "~!@#$%^&*()-_=+\\[{\\]}:\\\\|<\\.>/?";
        var results = new ArrayList<String>();
        boolean inString = false;
        StringBuilder current = new StringBuilder();
        int delimiter = '"';

        for (int pos = 0; pos < line.length(); pos++) {
            var c = line.codePointAt(pos);
            if (Character.isWhitespace(c) && !inString) {
                // It is possible for current to be empty when getting here, so
                // we want to handle all whitespace here so as not to add
                // spaces into the token list after other tokens are added.
                if (!current.isEmpty()) {
                    results.add(current.toString());
                }
                current.setLength(0);

            } else if (Character.isWhitespace(c) && inString) {
                current.appendCodePoint(c);

            } else if (!inString && (c == '"' || c == '\'')) {
                inString = true;
                if (current.isEmpty()) {
                    current.appendCodePoint('"'); // Mark the beginning.
                }
                delimiter = c;

            } else if (c == delimiter) {
                inString = false;

            } else if (!inString && symbols.contains(Character.toString(c))) {
                // At this point we have a symbol we have to do something about.
                if (!current.isEmpty()) {
                    results.add(current.toString());
                    current.setLength(0);
                }
                switch (c) {
                    // Doubles:
                    case '(' -> { if (checkNextChar(line, pos, ')')) { results.add("()"); pos++; } else { results.add("("); }}
                    case '[' -> { if (checkNextChar(line, pos, ']')) { results.add("[]"); pos++; } else { results.add("["); }}
                    case '@' -> { if (checkNextChar(line, pos, '@')) { results.add("@@"); pos++; } else { results.add("@"); }}
                    case '$' -> { if (checkNextChar(line, pos, '$')) { results.add("$$"); pos++; } else { results.add("$"); }}
                    case '-' -> { if (checkNextChar(line, pos, '>')) { results.add("->"); pos++; } else { results.add("-"); }}
                    case '_' -> { if (checkNextChar(line, pos, '_')) { results.add("__"); pos++; } else { results.add("_"); }}
                    case '<' -> { if (checkNextChar(line, pos, '-')) { results.add("<-"); pos++; } else { results.add("<"); }}
                    case '>' -> { if (checkNextChar(line, pos, '>')) { results.add(">>"); pos++; } else { results.add(">"); }}

                    // Complex:
                    case '?' -> {
                        if (checkNextChar(line, pos, ':')) {
                            results.add("?:"); pos++;
                        } else if (checkNextChar(line, pos, '<')) {
                            results.add("?<"); pos++;
                        } else if (checkNextChar(line, pos, '>')) {
                            results.add("?>"); pos++;
                        } else {
                            results.add("?");
                        }
                    }

                    // Singles:
                    default -> results.add(Character.toString(c));
                }

            } else {
                current.appendCodePoint(c);
            }
        }
        if (!current.isEmpty()) {
            results.add(current.toString());
            if (inString) {
                warnings.add("Unclosed string at " + dotPath + ":" + lineNo + "\t" + line);
            }
        }
        return results;
    }


    private static boolean checkNextChar(String line, int pos, int codePoint) {
        return pos < line.length() - 1 && line.codePointAt(pos + 1) == codePoint;
    }
}
