package nz.bradley.neil.scandi;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Token {

    public Type type;
    public String value;

    public Token(Type type) {
        this.type = type;
        value = null;
    }

    public Token(Type type, String value) {
        this.type = type;
        this.value = value;
    }

    public enum Type {
        NAME("N"),
        DEPTH("D"),

        NUMBER("V"),
        HEXADECIMAL("#"),
        STRING("\""),
        BLOB("__"),
        NULL("()"),

        WITH_OPEN("{"),
        WITH_CLOSE("}"),
        NEGATIVE_OPEN("("),
        NEGATIVE_CLOSE(")"),

        OPERATOR_DOT("."),
        OPERATOR_COUNT("!"),
        OPERATOR_INDEX_OPEN("["),
        OPERATOR_INDEX_CLOSE("]"),
        OPERATOR_COLLECTION("[]"),

        DECLARATION_VARIABLE("$"),
        DECLARATION_VARIABLE_UP("$$"),
        DECLARATION_FUNCTION("@"),
        DECLARATION_FUNCTION_UP("@@"),
        DECLARATION_LABEL("\\"),
        ADDRESS("_"),

        COMPARATOR_EQUALS("?"),
        COMPARATOR_EQUALS_REFERENCE("?:"),
        COMPARATOR_LESS("<"),
        COMPARATOR_GREATER(">"),
        COMPARATOR_LESS_EQUAL("?<"),
        COMPARATOR_GREATER_EQUAL("?>"),

        ASSIGNMENT_COPY("="),
        ASSIGNMENT_REFERENCE(":"),

        OPERATOR_COMPLEMENT("~"),
        OPERATOR_AND("&"),
        OPERATOR_OR("|"),
        OPERATOR_XOR("^"),
        OPERATOR_ADD("+"),
        OPERATOR_SUB("-"),
        OPERATOR_MUL("*"),
        OPERATOR_DIV("/"),
        OPERATOR_MOD("%"),
        OPERATOR_SHL("<-"),
        OPERATOR_SHR("->"),
        OPERATOR_SSR(">>");

        public final String value;

        Type(String value) {
            this.value = value;
        }
    }

    /**
     *
     * @param data  A map containing all the lines for each file, with
     *              comments and empty lines removed.
     *
     * @return  The file contents, tokenized.
     */
    public static List<Token> tokenize(String relativeDotPath, List<String> data) {
        var name = relativeDotPath.contains(".")
                ? relativeDotPath.substring(relativeDotPath.lastIndexOf("."))
                : relativeDotPath;
        var tokens = new ArrayList<>(List.of(new Token(Type.DEPTH, "0"), new Token(Type.NAME, name)));
        var warnings = new ArrayList<String>();
        int count = 0;

        for (var line: data) {
            count++;
            tokens.add(new Token(Type.DEPTH,getDepth(line)));
            var rawTokens = splitWithStrings(line.stripLeading(), relativeDotPath, count);

        }
        return tokens;
    }

    private static String getDepth(String line) {
        // Files have depth of 1 - globals are at 0.
        return "" + (1 + line.length() - line.stripLeading().length());
    }

    private static List<String> splitWithStrings(String line, String dotPath, int lineNo) {
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
            if (!inString) {
                results.add(current.toString());
            } else {
                throw new RuntimeException("Unclosed string at " + dotPath + ":" + lineNo + "\t" + line);
            }
        }
        return results;
    }

    private static boolean checkNextChar(String line, int pos, int codePoint) {
        return pos < line.length() - 1 && line.codePointAt(pos + 1) == codePoint;
    }
}
