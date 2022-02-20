package nz.bradley.neil.scandi.analysers;

import nz.bradley.neil.scandi.language.Token;

import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

public class Syntax {

    public static Scope analyse(
            List<List<Token>> tokens,
            List<String> warnings,
            List<String> errors
    ) {
        Scope root = new Scope(null, new Token(Token.Type.DEPTH, null, "", -1, -1));

        // Each phrase within tokens needs to be checked for correct syntax,
        // and the scope tree built as we go.
        tokens.forEach(phrase -> {
            var depth = phrase.remove(0);
            addMissingAssignment(phrase, warnings, errors);
            condenseDeclarations(phrase, warnings, errors);
        });
        return root;
    }

    private static void addMissingAssignment(
            final List<Token> phrase,
            final List<String> warnings,
            final List<String> errors
    ) {
        var id = phrase.get(0);
        var op = phrase.get(phrase.size() - 1);
        if (Token.isOperator(op) && id.type() == Token.Type.IDENTIFIER) {
            phrase.add(0, id);
            phrase.add(new Token(Token.Type.COPY, null, op.file(), op.line(), op.position()));
        }
    }

    private static void condenseDeclarations(
            List<Token> phrase,
            List<String> warnings,
            List<String> errors
    ) {
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
