from flask import Flask, request, Response
import subprocess
import os

app = Flask(__name__)

# ✅ IMPROVED LANGUAGE DETECTION
def detect_language(code):
    code_lower = code.lower()

    # ---- C# (detect but mark unsupported) ----
    if "using system" in code_lower or "console.writeline" in code_lower:
        return "C#"

    # ---- C ----
    if "#include <stdio.h>" in code_lower or "printf" in code_lower:
        return "C"

    # ---- C++ ----
    if "#include <iostream>" in code_lower or "cout" in code_lower:
        return "C++"

    # ---- Java ----
    if "public class" in code_lower or "system.out.println" in code_lower:
        return "Java"

    # ---- Python ----
    if "def " in code_lower or "print(" in code_lower:
        return "Python"

    # ❌ REMOVE WRONG FALLBACK (=)
    return "Unknown"


@app.route("/")
def home():
    return open("dashboard.html", "r", encoding="utf-8").read()


@app.route("/run", methods=["POST"])
def run_code():
    try:
        data = request.get_json()
        code = data.get("code", "")

        # STEP 1: DETECT LANGUAGE
        lang = detect_language(code)

        # ❌ HANDLE UNSUPPORTED LANGUAGES
        if lang not in ["C", "C++", "Java", "Python"]:
            return Response(
                f"❌ Unsupported Language Detected: {lang}\n\n"
                f"✔ Supported: C, C++, Java, Python",
                mimetype="text/plain"
            )

        # STEP 2: EXECUTABLE MAP
        if os.name == "nt":
            executables = {
                "C": "parser.exe",
                "C++": "parser_cpp.exe",
                "Java": "parser_java.exe",
                "Python": "parser_python.exe"
            }
        else:
            executables = {
                "C": "./parser",
                "C++": "./parser_cpp",
                "Java": "./parser_java",
                "Python": "./parser_python"
            }

        exe = executables.get(lang)

        if not exe:
            return Response(
                f"❌ Parser not found for {lang}",
                mimetype="text/plain"
            )

        # STEP 3: RUN PARSER
        process = subprocess.Popen(
            [exe],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        output, error = process.communicate(code + "\n~\n")

        if error.strip():
            return Response("Error:\n" + error, mimetype="text/plain")

        return Response(
            f"✅ Language Detected: {lang}\n\n" + output,
            mimetype="text/plain"
        )

    except Exception as e:
        return Response("Server Error: " + str(e), mimetype="text/plain")


if __name__ == "__main__":
    app.run(debug=True)