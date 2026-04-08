FROM python:3.10-slim

WORKDIR /app

# Install dependencies
RUN apt-get update && apt-get install -y \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy source
COPY . .

# Set Python path
ENV PYTHONPATH=/app/PythonAPI:$PYTHONPATH

# Default command
CMD ["python3", "-m", "pytest", "PythonAPI/test"]
