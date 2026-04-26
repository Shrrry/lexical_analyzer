from flask import Flask, request, Response
import subprocess
import os

app = Flask(__name__)


# ---------------- LANGUAGE DETECTION ----------------
def detect_language(code):
    code_lower = code.lower()

    if "#include <stdio.h>" in code_lower or "printf" in code_lower:
        return "C"

    elif "#include <iostream>" in code_lower or "cout" in code_lower:
        return "C++"

    elif "public class" in code_lower or "system.out.println" in code_lower:
        return "Java"

    elif "def " in code_lower or "print(" in code_lower:
        return "Python"

    return "Unknown"


# ---------------- RUN EXECUTABLE ----------------
def run_executable(exe_name, code):
    process = subprocess.Popen(
        [exe_name],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output, error = process.communicate(code + "\n~\n")

    if error.strip():
        return "ERROR:\n" + error

    return output


@app.route("/")
def home():
    return open("dashboard.html", encoding="utf-8").read()


@app.route("/run", methods=["POST"])
def run_code():
    try:
        data = request.get_json()
        code = data["code"]

        language = detect_language(code)

        if language == "Unknown":
            return Response(
                "Unsupported Language",
                mimetype="text/plain"
            )

        # Windows / Linux compatibility
        if os.name == "nt":
            error_checkers = {
                "C": "error_c.exe",
                "C++": "error_cpp.exe",
                "Java": "error_java.exe",
                "Python": "error_python.exe"
            }

            lexers = {
                "C": "parser_c.exe",
                "C++": "parser_cpp.exe",
                "Java": "parser_java.exe",
                "Python": "parser_python.exe"
            }

        else:
            error_checkers = {
                "C": "./error_c",
                "C++": "./error_cpp",
                "Java": "./error_java",
                "Python": "./error_python"
            }

            lexers = {
                "C": "./parser_c",
                "C++": "./parser_cpp",
                "Java": "./parser_java",
                "Python": "./parser_python"
            }

        # ---------------- STEP 1 ERROR CHECKER ----------------
        error_output = run_executable(
            error_checkers[language],
            code
        )

        if ("Lexical Error" in error_output or
            "Syntax Errors" in error_output or
            "Semantic Errors" in error_output):

            return Response(
                f" Language Detected: {language}\n\n"
                f"{error_output}",
                mimetype="text/plain"
            )

        # ---------------- STEP 2 LEXICAL PARSER ----------------
        parser_output = run_executable(
            lexers[language],
            code
        )

        return Response(
            f" Language Detected: {language}\n\n"
            f"No Errors Found\n\n"
            f"{parser_output}",
            mimetype="text/plain"
        )

    except Exception as e:
        return Response(
            "Server Error: " + str(e),
            mimetype="text/plain"
        )


if __name__ == "__main__":
    app.run(debug=True)