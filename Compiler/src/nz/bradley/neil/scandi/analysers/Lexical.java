package nz.bradley.neil.scandi.analysers;

import nz.bradley.neil.scandi.language.Token;

import java.util.ArrayList;
import java.util.List;

public class Lexical {

    /**
     * <p>
     *     Converts a provided code listing into a series of
     *     {@link Token}s.
     * </p>
     *
     * @param relativeDotPath   The file path, in dot format matching
     *                          Scandi's include format.
     * @param data              The lines of the file.
     * @param warnings          A list to put any warnings generated in.
     * @param errors            A list to put any errors identified in.
     *
     * @return  The file contents, as phrases of Tokens.
     */
    public static List<List<Token>> analyse(
            String relativeDotPath,
            List<String> data,
            List<String> warnings,
            List<String> errors
    ) {
        List<List<Token>> tokens = new ArrayList<>();

        for (int lineNo = 1; lineNo <= data.size(); lineNo++) {
            String line = data.get(lineNo - 1);

            // Strip comments, but keep line numbers intact.
            if (line.contains("`")) {
                line = line.substring(0, line.indexOf("`")).trim();
            }
            if (!line.isBlank()) {
                var phrase = new ArrayList<Token>();
                int depth = getDepth(line);
                phrase.add(new Token(Token.Type.DEPTH, null, relativeDotPath, lineNo, depth));
                phrase.addAll(splitWithStrings(line.stripLeading(), relativeDotPath, lineNo, depth, warnings, errors));
                tokens.add(phrase);
            }
        }

        return tokens;
    }


    private static int getDepth(String line) {
        // Files have depth of 0 - globals are at -1.
        return line.length() - line.stripLeading().length();
    }


    private static List<Token> splitWithStrings(String line, String dotPath, int lineNo, int depth, List<String> warnings, List<String> errors) {
        var results = new ArrayList<Token>();
        boolean inString = false;
        boolean isContinuation = false;
        StringBuilder current = new StringBuilder();
        char delimiter = '"';
        int tokenStart = -1;

        for (int pos = 0; pos < line.length(); pos++) {
            var c = line.charAt(pos);

            // 1. Check for the completion of a string/token.
            if (!inString && isContinuation && Character.isWhitespace(c)) {
                isContinuation = false;      // There is no continuation.
                if (!current.isEmpty()) {
                    var s = current.toString();
                    try {
                        results.add(new Token(Token.Type.of(s), s, dotPath, lineNo, depth + tokenStart));
                    } catch (IllegalArgumentException iae) {
                        errors.add(iae.getMessage() + " at " + dotPath + "@" + lineNo + ":" + tokenStart);
                    }
                    current.setLength(0);
                    tokenStart = -1;
                }

            // 2. Check for the start of a string.
            } else if (!inString && (c == '"' || c == '\'')) {
                inString = true;
                if (!isContinuation) {
                    current.append('"');
                    isContinuation = true;
                    tokenStart = pos;
                }
                delimiter = c;

            // 3. Check for the end of a string.
            } else if (inString) {
                if (c == delimiter) {
                    inString = false;
                } else {
                    current.append(c);
                }

            // 4. Check the token.
            } else if (!Character.isWhitespace(c)) {
                var type = Token.Type.of(line.substring(pos));
                String value;
                switch (type) {
                    case DEPTH -> errors.add("Illegal character ';' in " + dotPath + "@" + lineNo + ":" + (depth + pos));
                    case NUMBER -> {
                        value = getBlock("([^\\p{Alnum},])", line, pos);
                        results.add(new Token(type, value, dotPath, lineNo, pos));
                        pos += value.length() - 1;
                    }
                    case IDENTIFIER -> {
                        value = getBlock("([^\\p{Alnum}])", line, pos);
                        results.add(new Token(type, value, dotPath, lineNo, pos));
                        pos += value.length() - 1;
                    }
                    case HEX -> {
                        pos++;
                        value = getBlock("([^\\p{XDigit}])", line, pos);
                        results.add(new Token(type, value, dotPath, lineNo, pos));
                        pos += value.length();
                    }
                    default -> {
                        results.add(new Token(type,  null, dotPath, lineNo, pos));
                        pos += type.getSymbol().length() - 1;
                    }
                }
            }
        }
        if (!current.isEmpty()) {
            var s = current.toString();
            try {
                results.add(new Token(Token.Type.of(s), s, dotPath, lineNo, depth + tokenStart));
            } catch (IllegalArgumentException iae) {
                errors.add(iae.getMessage() + " at " + dotPath + "@" + lineNo + ":" + tokenStart);
            }
            if (inString) {
                warnings.add("Unclosed string at " + dotPath + ":" + lineNo + "\t" + line);
            }
        }
        return results;
    }


    private static String getBlock(String regex, String line, int pos) {
        int end = line.substring(pos).replaceFirst(regex, " ").indexOf(" ");
        if (end < 0) {
            end = line.length();
        } else {
            end += pos;
        }
        return line.substring(pos, end);
    }
}
