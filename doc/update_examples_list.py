import os
import re

def extract_example_description(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        match = re.search(r'#\s*Example description\n#(.*?)(?=\n\n|$)', content, re.DOTALL)
        if match:
            description = match.group(1)
            description_lines = [line.strip('#').lstrip() for line in description.split('\n')]
            return '\n'.join(description_lines).strip()
    return None

def create_rst_file(output_file, directory_path):
    with open(output_file, 'w') as rst_file:
        for file_name in os.listdir(directory_path):
            if file_name.endswith('.py'):
                file_path = os.path.join(directory_path, file_name)
                example_description = extract_example_description(file_path)
                
                if example_description:
                    # Write to rst file
                    relative_path = os.path.relpath(file_path, directory_path)
                    rst_file.write(f"\nExample: {file_name[:-3]}\n{'-' * (10 + len(file_name) - 3)}\n\n")
                    rst_file.write(f"{example_description}\n\n")
                    rst_file.write(f".. literalinclude:: ../../../python/examples/{relative_path}\n   :language: py\n\n")

if __name__ == "__main__":
    create_rst_file("./rst/python/examples.rst", "../python/examples")
