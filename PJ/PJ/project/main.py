from flask import Flask, request, jsonify,render_template
import subprocess
import os

app = Flask(__name__)

# 存储上传文件的文件夹
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# 设置允许的文件扩展名
ALLOWED_EXTENSIONS = {'yo'}

# 检查文件扩展名是否合法
def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

# 存储分步结果的缓存
step_results = []

@app.route('/')
def index():
    # 简单返回主页面提示
    return render_template('index.html')

@app.route('/upload', methods=['POST'])
def upload_file():
    """上传文件接口"""
    if 'file' not in request.files:
        return jsonify({'error': 'No file part'}), 400
    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400
    if file and allowed_file(file.filename):
        file_path = os.path.join(UPLOAD_FOLDER, file.filename)
        file.save(file_path)
        return jsonify({'message': 'File uploaded successfully', 'file_path': file_path}), 200
    return jsonify({'error': 'Invalid file format'}), 400

@app.route('/run', methods=['POST'])
def run_simulator():
    """运行模拟器接口"""
    global step_results
    file_path = request.json.get('file_path')  # 获取文件路径
    mode = request.json.get('mode', 'result')  # 获取模式：result 或 step

    if not file_path or not os.path.exists(file_path):
        return jsonify({'error': 'File not found'}), 400

    # 根据模式选择可执行文件
    executable = './cpu' if mode == 'result' else './cpuins'

    try:
        if mode == 'result':
            # 直接展示结果
            result = subprocess.run(
                [executable],
                input=open(file_path,'rb').read(),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=True
            )
            output = result.stdout.decode()
            if result.returncode != 0:
                return jsonify({'error': 'Simulator error', 'details': result.stderr}), 500
            return jsonify({'message': 'Execution successful', 'output': output}), 200

        elif mode == 'step':
            # 分步展示结果，初始化分步缓存
            process = subprocess.run(
                [executable],
                input=open(file_path,'rb').read(),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=True
            )
            step_output= process.stdout.decode()
            step_results = step_output.split('-')
            return jsonify({'message': 'Step mode initialized', 'steps': len(step_results)}), 200

    except subprocess.CalledProcessError as e:
        return jsonify({'error': 'Simulator error', 'details': str(e)}), 500

@app.route('/step', methods=['POST'])
def get_next_step():
    """获取下一步结果接口"""
    global step_results
    step_index = request.json.get('step_index', 0)

    if step_index < len(step_results):
        return jsonify({'message': 'Step result', 'output': step_results[step_index]}), 200
    return jsonify({'message': 'No more steps', 'output': ''}), 400

if __name__ == '__main__':
    app.run(debug=True)
