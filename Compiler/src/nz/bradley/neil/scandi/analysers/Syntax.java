package nz.bradley.neil.scandi.analysers;

import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

public class Syntax {

    public static boolean analyse(
            String dotPath,
            List<String> lexemes,
            List<String> warnings,
            List<String> errors
    ) {
        StringBuilder currentLine = new StringBuilder();

        // Convert lexemes to tokens, and check syntax.
        for (String lexeme : lexemes) {
            // 1. Check prefixes.
            if (lexeme.startsWith(";")) {
                // Depth
                if (!currentLine.toString().isBlank()) {
                    if (!checkStatement(currentLine.toString())) {
                        errors.add("Invalid syntax in " + dotPath + ": " + currentLine);
                    }
                    currentLine.setLength(0);
                }

            } else if (lexeme.startsWith("\"")) {
                // String
                currentLine.append("STR-");

            } else if (currentLine.toString().endsWith("ADR-") && isHexadecimal(lexeme)) {
                currentLine.append("HEX-");

            } else if (isNumber(lexeme)) {
                currentLine.append("NUM-");

            } else if (isIdentifier(lexeme)) {
                currentLine.append("ID-");

            } else {
                // This should just leave operators.
                switch (lexeme) {
                    case "()" -> currentLine.append("NUL-");
                    case "[]" -> currentLine.append("ARR-");
                    case ".[]" -> currentLine.append("ME-ARR-");
                    case "<-" -> currentLine.append("SHL-");
                    case "->" -> currentLine.append("SHR-");
                    case ">>" -> currentLine.append("SSR-");
                    case "?:" -> currentLine.append("EQR-");
                    case "?<" -> currentLine.append("LTE-");
                    case "?>" -> currentLine.append("GTE-");
                    case "__" -> currentLine.append("BIN-");
                    case "@@", "@" -> currentLine.append("FNC-");
                    case "$$", "$" -> currentLine.append("VAR-");
                    case "_" -> currentLine.append("ADR-");
                    case "{" -> currentLine.append("WTL-");
                    case "}" -> currentLine.append("WTR-");
                    case "\\" -> currentLine.append("LBL-");
                    case "=" -> currentLine.append("ASS-");
                    case ":" -> currentLine.append("REF-");
                    case "?" -> currentLine.append("EQ-");
                    case "<" -> currentLine.append("LT-");
                    case ">" -> currentLine.append("GT-");
                    case "~" -> currentLine.append("COM-");
                    case "&" -> currentLine.append("AND-");
                    case "|" -> currentLine.append("OR-");
                    case "^" -> currentLine.append("XOR-");
                    case "+" -> currentLine.append("ADD-");
                    case "-" -> currentLine.append("SUB-");
                    case "*" -> currentLine.append("MUL-");
                    case "/" -> currentLine.append("DIV-");
                    case "%" -> currentLine.append("MOD-");
                    case ".." -> currentLine.append("ME-DOT-");
                    case "." -> currentLine.append("DOT-");
                    case ".[" -> currentLine.append("ME-INL-");
                    case "[" -> currentLine.append("INL-");
                    case "]" -> currentLine.append("INR-");
                    case "(" -> currentLine.append("NGL-");
                    case ")" -> currentLine.append("NGR-");
                    case "#" -> currentLine.append("HEX-");
                    default -> warnings.add("Unknown symbol in " + dotPath + ":" + lexeme);
                }
            }
        }
        if (!currentLine.toString().isBlank()) {
            if (!checkStatement(currentLine.toString())) {
                errors.add("Invalid syntax in " + dotPath + ": " + currentLine);
            }
            currentLine.setLength(0);
        }
        return errors.isEmpty();
    }

    private static boolean isHexadecimal(String isHex) {
        return isHex != null && isHex.matches("\\p{XDigit}+");
    }

    private static boolean isNumber(String isNo) {
        // Note that this allows ',' but doesn't check how many there were.
        return isNo != null && isNo.matches("(\\p{Digit}|,)+");
    }

    private static boolean isIdentifier(String isId) {
        return isId != null && !isId.isBlank() && Character.isAlphabetic(isId.codePointAt(0));
    }

    private static boolean checkStatement(String line) {
        // These are non-reducible
        if (line.startsWith("WTL-")) {
            // With statement
            return checkWith(line.substring(4));

        } else if (line.matches("((NUL-)|(ARR-)|(VAR-ID-)*)FNC-ID-$")) {
            // Function declaration
            return true;

        }

        // Reduce references where appropriate.
        line = maskOperators(line);
        AtomicReference<Boolean> complexOK = new AtomicReference<>(true);
        line = reduceComplexIDs(line, complexOK);
        if (!complexOK.get()) {
            return false;
        }

        if ("NUL-".equals(line)) {
            // Empty if or else block
            return true;

        } else if ("LBL-ID-".equals(line)) {
            // Label declaration
            return true;

        } else if (line.startsWith("LBL-ID-")) {
            // Label followed by statement
            return checkStatement(line.substring(7));

        } else if ("ID-".equals(line)) {
            // Jump to label
            return true;

        } else if ("VAR-ID-".equals(line)) {
            // Empty variable declaration
            return true;

        } else if (line.startsWith("VAR-ID-")) {
            // Variable declaration with assignment
            return checkAssignment(line.substring(7));

        } else if (line.startsWith("ADR-HEX-")) {
            // Address assignment
            return checkAssignment(line.substring(8));

        } else if (line.matches("ID-.*(ASS|REF)-")) {
            return checkAssignment(line.substring(3));

        } else if (line.matches(".*((EQR?)|([GL]TE?))-$")) {
            // Comparator
            return checkExpression(line.substring(0,line.substring(0, line.length() - 1).lastIndexOf('-') + 1));

        } else if (line.matches(".*ID-$")) {
            // Function call
            return checkExpression(line.substring(0, line.length() - 3));

        } else if (line.matches("ID-.*OP-$")) {
            return checkExpression(line);
        }

        return false;
    }

    private static boolean checkWith(String line) {
        if (!line.endsWith("WTR-")) {
            return false;
        }
        line = line.substring(0, line.length() - 4);
        return line.matches("ID-(DOT-ID-)*");
    }

    private static boolean checkAssignment(String assignment) {
        if (assignment.endsWith("ASS-") || assignment.endsWith("REF-")) {
            return checkExpression(assignment.substring(0,assignment.length() - 4));
        }
        return false;
    }

    private static boolean checkExpression(String expression) {
        if (expression.startsWith("OP-")) {
            return false;
        } else return expression.matches("(((NU[LM])|(ME)|(STR)|(ID)|(OP)|((ADR-)?HEX)|)-)*");
    }

    private static String reduceComplexIDs(String line, AtomicReference<Boolean> complexOK) {
        while (line.contains("ID-DOT-ID-") || line.contains("INL-")) {
            // Remove excessive DOT references
            line = line.replaceAll("ID-DOT-ID-", "ID-");
            line = line.replaceAll("ME-DOT-ID-", "ME-");
            // Remove INDEXES. These need to be checked for validity.
            if (line.contains("INL-")) {
                int end = line.indexOf("INR-");
                if (end >= 0) {
                    int start = line.substring(0, end).lastIndexOf("INL-");
                    if (start >= 0) {
                        if (!checkExpression(line.substring(start + 4, end))) {
                            complexOK.set(false);
                        }
                        line = line.substring(0, start) + line.substring(end + 4);
                    } else {
                        line = "";
                        complexOK.set(false);
                    }
                } else {
                    line = "";
                    complexOK.set(false);
                }
            }
        }
        return line;
    }

    private static String maskOperators(String line) {
        return line.replaceAll("(SHL)|(SHR)|(SSR)|(COM)|(AND)|(XOR)|(ADD)|(SUB)|(MUL)|(DIV)|(MOD)", "OP").replaceAll("OR", "OP");
    }
}
