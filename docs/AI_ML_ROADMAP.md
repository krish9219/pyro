# Pyro AI/ML/DL Roadmap

**Goal:** Make Pyro the first language with AI, ML, and Deep Learning built into the standard library. Zero pip install. Zero dependencies. Just `import` and go.

---

## Phase 1 — Tensor Engine (Foundation)

**Timeline:** 2-3 weeks
**Priority:** Critical — everything else depends on this
**Module:** `tensor`

### What It Does
A NumPy-equivalent tensor library that compiles to native C++ with optional BLAS/LAPACK acceleration.

### API Design

```pyro
import tensor

# Creation
a = tensor.new([1.0, 2.0, 3.0])
b = tensor.zeros(3, 3)
c = tensor.ones(4, 4)
d = tensor.random(100, 100)
e = tensor.range(0.0, 1.0, 0.1)
f = tensor.eye(5)

# Properties
print(a.shape)    # [3]
print(b.ndim)     # 2
print(c.size)     # 16
print(d.dtype)    # "float64"

# Reshape
g = a.reshape(1, 3)
h = d.transpose()
i = d.flatten()

# Math operations
j = a + b
k = a * 2.0
l = tensor.dot(a, a)
m = tensor.matmul(b, c)
n = tensor.sum(d)
o = tensor.mean(d)
p = tensor.std(d)
q = tensor.min(d)
r = tensor.max(d)

# Element-wise
s = tensor.sqrt(a)
t = tensor.exp(a)
u = tensor.log(a)
v = tensor.abs(a)
w = tensor.pow(a, 2.0)

# Linear algebra
det = tensor.det(b)
inv = tensor.inverse(b)
vals = tensor.eigenvalues(b)
svd_result = tensor.svd(b)

# Slicing
row = d.row(0)
col = d.col(0)
sub = d.slice(0, 10, 0, 10)

# Comparison
mask = a > 2.0
filtered = a.where(mask)
```

### Implementation Plan
1. Create `TensorData` struct in codegen: `std::vector<double>` + shape info
2. Implement all creation functions (zeros, ones, random, eye, range)
3. Implement element-wise operations (+, -, *, /)
4. Implement matrix multiply using nested loops (baseline)
5. Add BLAS linking for matmul/dot acceleration (optional, detected at compile time)
6. Implement reshape, transpose, flatten, slice
7. Implement reduction operations (sum, mean, std, min, max)
8. Implement linear algebra (det, inverse, eigenvalues via QR iteration)
9. Add GPU support via CUDA (optional future extension)

### C++ Backend
```cpp
namespace pyro_tensor {
    struct Tensor {
        std::vector<double> data;
        std::vector<int64_t> shape;
        int64_t ndim() const { return shape.size(); }
        int64_t size() const { /* product of shape */ }
        Tensor reshape(std::vector<int64_t> new_shape) const;
        Tensor transpose() const;
        // ... all operations
    };
    Tensor matmul(const Tensor& a, const Tensor& b);
    Tensor zeros(int64_t rows, int64_t cols);
    // ...
}
```

### Tests Needed
- Creation: zeros, ones, random, eye, range (5 tests)
- Shape: reshape, transpose, flatten (3 tests)
- Math: add, sub, mul, div, matmul, dot (6 tests)
- Reduction: sum, mean, std, min, max (5 tests)
- Linear algebra: det, inverse, eigenvalues (3 tests)
- Edge cases: empty tensor, scalar tensor, 1D/2D/3D (3 tests)
- **Total: 25 tests**

---

## Phase 2 — Classical Machine Learning

**Timeline:** 3-4 weeks (after Phase 1)
**Priority:** High — this is the scikit-learn killer pitch
**Module:** `ml`

### What It Does
Complete classical ML library: preprocessing, training, evaluation, model persistence. All compiled to native C++ — faster than scikit-learn.

### API Design

```pyro
import ml

# Load data
df = ml.load_csv("housing.csv")

# Preprocessing
df = ml.fill_missing(df, "mean")
df = ml.normalize(df)
df = ml.one_hot(df, "category")
train, test = ml.split(df, 0.8)

# === REGRESSION ===
model = ml.linear_regression(train, target="price")
model = ml.ridge(train, target="price", alpha=0.1)
model = ml.lasso(train, target="price", alpha=0.1)
model = ml.polynomial(train, target="price", degree=3)

# === CLASSIFICATION ===
model = ml.logistic(train, target="label")
model = ml.decision_tree(train, target="label", max_depth=10)
model = ml.random_forest(train, target="label", trees=100)
model = ml.knn(train, target="label", k=5)
model = ml.svm(train, target="label", kernel="rbf")
model = ml.naive_bayes(train, target="label")
model = ml.gradient_boost(train, target="label", trees=100, lr=0.1)

# === CLUSTERING ===
labels = ml.kmeans(data, k=3)
labels = ml.dbscan(data, eps=0.5, min_points=5)
labels = ml.hierarchical(data, k=3)

# === DIMENSIONALITY REDUCTION ===
reduced = ml.pca(data, components=2)
reduced = ml.tsne(data, components=2, perplexity=30)

# === PREDICT & EVALUATE ===
predictions = model.predict(test)
print("R2: {ml.r2_score(predictions, test.price)}")
print("MSE: {ml.mse(predictions, test.price)}")
print("MAE: {ml.mae(predictions, test.price)}")
print("Accuracy: {ml.accuracy(predictions, test.label)}")
print("Precision: {ml.precision(predictions, test.label)}")
print("Recall: {ml.recall(predictions, test.label)}")
print("F1: {ml.f1_score(predictions, test.label)}")
cm = ml.confusion_matrix(predictions, test.label)

# === CROSS VALIDATION ===
scores = ml.cross_validate(model, data, target="label", folds=5)
print("Mean accuracy: {ml.mean(scores)}")

# === FEATURE IMPORTANCE ===
importance = model.feature_importance()
for name, score in importance
    print("{name}: {score}")

# === SAVE/LOAD ===
ml.save(model, "model.pyro")
loaded = ml.load("model.pyro")
```

### Implementation Plan (by model)

**Week 1 — Foundation + Linear Models:**
1. Data loading and DataFrame integration
2. Train/test split with random shuffle
3. Preprocessing: normalize, standardize, one_hot, fill_missing
4. Linear regression (normal equation + gradient descent)
5. Ridge regression (L2 regularization)
6. Lasso regression (L1 with coordinate descent)
7. Logistic regression (gradient descent)
8. Evaluation metrics: MSE, MAE, R2, accuracy, precision, recall, F1

**Week 2 — Tree Models:**
9. Decision tree (ID3/C4.5 algorithm, Gini/entropy splitting)
10. Random forest (bagging + random feature selection)
11. Gradient boosting (sequential tree building)
12. Feature importance (from tree models)
13. Cross-validation (k-fold)

**Week 3 — Distance & Kernel Models:**
14. KNN (brute force + optional KD-tree)
15. SVM (SMO algorithm for linear + RBF kernel)
16. Naive Bayes (Gaussian + Multinomial)
17. K-Means clustering
18. DBSCAN clustering

**Week 4 — Dimensionality + Polish:**
19. PCA (eigendecomposition of covariance matrix)
20. t-SNE (Barnes-Hut approximation)
21. Model save/load (binary serialization)
22. Confusion matrix, classification report
23. Polynomial regression

### C++ Backend Structure
```cpp
namespace pyro_ml {
    struct Dataset { /* tensor-based columns */ };
    struct Model {
        virtual Tensor predict(const Dataset& data) = 0;
        virtual void save(const std::string& path) = 0;
    };
    struct LinearRegression : Model { /* weights, bias, fit(), predict() */ };
    struct DecisionTree : Model { /* nodes, split(), predict() */ };
    struct RandomForest : Model { /* vector of trees */ };
    // ...
}
```

### Tests Needed
- Preprocessing: normalize, one_hot, fill_missing, split (4 tests)
- Linear models: linear_regression, ridge, lasso, logistic (4 tests)
- Tree models: decision_tree, random_forest, gradient_boost (3 tests)
- Distance models: knn, svm, naive_bayes (3 tests)
- Clustering: kmeans, dbscan (2 tests)
- Dimensionality: pca (1 test)
- Evaluation: all metrics (5 tests)
- Save/load: serialize and deserialize (1 test)
- Cross-validation (1 test)
- **Total: 24 tests**

---

## Phase 3 — Neural Networks & Deep Learning

**Timeline:** 6-8 weeks (after Phase 1)
**Priority:** High — this is the TensorFlow/PyTorch killer pitch
**Module:** `nn`

### What It Does
A complete neural network framework. Define, train, evaluate, and deploy deep learning models — all compiled to native C++ with optional CUDA GPU acceleration.

### API Design

```pyro
import nn

# === SEQUENTIAL MODEL ===
model = nn.sequential([
    nn.dense(784, 256, "relu"),
    nn.batchnorm(256),
    nn.dropout(0.3),
    nn.dense(256, 128, "relu"),
    nn.dropout(0.2),
    nn.dense(128, 10, "softmax")
])

# Compile
model.compile(
    optimizer="adam",
    loss="cross_entropy",
    lr=0.001
)

# Train
history = model.fit(
    x_train, y_train,
    epochs=20,
    batch=64,
    validation=(x_val, y_val)
)

# Training output:
# Epoch 1/20 - loss: 2.301 - acc: 0.112 - val_loss: 2.290 - val_acc: 0.135
# Epoch 2/20 - loss: 1.854 - acc: 0.432 - val_loss: 1.721 - val_acc: 0.489
# ...

# Evaluate
loss, accuracy = model.evaluate(x_test, y_test)
print("Test accuracy: {accuracy}")

# Predict
prediction = model.predict(single_image)
print("Predicted digit: {prediction.argmax()}")

# Save/Load
nn.save(model, "mnist.pyro")
loaded = nn.load("mnist.pyro")


# === CONVOLUTIONAL NEURAL NETWORK ===
cnn = nn.sequential([
    nn.conv2d(1, 32, kernel=3, stride=1, padding=1),
    nn.relu(),
    nn.maxpool(2),
    nn.conv2d(32, 64, kernel=3, stride=1, padding=1),
    nn.relu(),
    nn.maxpool(2),
    nn.flatten(),
    nn.dense(3136, 128, "relu"),
    nn.dropout(0.5),
    nn.dense(128, 10, "softmax")
])


# === RECURRENT NEURAL NETWORK ===
rnn = nn.sequential([
    nn.embedding(vocab_size, 128),
    nn.lstm(128, hidden=256, layers=2, dropout=0.3),
    nn.dense(256, vocab_size, "softmax")
])


# === TRANSFORMER ===
transformer = nn.sequential([
    nn.embedding(vocab_size, 512),
    nn.positional_encoding(512),
    nn.transformer_block(heads=8, dim=512, ff_dim=2048),
    nn.transformer_block(heads=8, dim=512, ff_dim=2048),
    nn.transformer_block(heads=8, dim=512, ff_dim=2048),
    nn.dense(512, vocab_size, "softmax")
])


# === CUSTOM TRAINING LOOP ===
optimizer = nn.adam(model.params(), lr=0.001)
loss_fn = nn.cross_entropy()

for epoch in 0..100
    for batch_x, batch_y in nn.batches(x_train, y_train, 32)
        predictions = model.forward(batch_x)
        loss = loss_fn(predictions, batch_y)
        gradients = nn.backward(loss)
        optimizer.step(gradients)
    print("Epoch {epoch}: loss = {loss}")


# === LEARNING RATE SCHEDULERS ===
scheduler = nn.cosine_decay(optimizer, total_steps=1000)
scheduler = nn.step_decay(optimizer, step_size=30, gamma=0.1)
scheduler = nn.warmup(optimizer, warmup_steps=100)


# === DATA AUGMENTATION ===
augmented = nn.augment(images, [
    nn.random_flip("horizontal"),
    nn.random_rotate(15),
    nn.random_crop(28, 28),
    nn.normalize(mean=0.5, std=0.5)
])


# === MODEL SUMMARY ===
model.summary()
# Layer              Output Shape    Params
# dense_1            (256,)          200,960
# batchnorm_1        (256,)          512
# dropout_1          (256,)          0
# dense_2            (128,)          32,896
# dropout_2          (128,)          0
# dense_3            (10,)           1,290
# Total params: 235,658
```

### Layer Types to Implement

| Layer | Purpose | Complexity |
|-------|---------|-----------|
| `nn.dense` | Fully connected | Simple |
| `nn.conv2d` | 2D convolution | Medium |
| `nn.maxpool` | Max pooling | Simple |
| `nn.avgpool` | Average pooling | Simple |
| `nn.flatten` | Reshape to 1D | Simple |
| `nn.dropout` | Regularization | Simple |
| `nn.batchnorm` | Normalization | Medium |
| `nn.layernorm` | Layer normalization | Medium |
| `nn.embedding` | Word embeddings | Simple |
| `nn.lstm` | Long short-term memory | Hard |
| `nn.gru` | Gated recurrent unit | Hard |
| `nn.attention` | Multi-head attention | Hard |
| `nn.transformer_block` | Full transformer block | Hard |
| `nn.positional_encoding` | Position info for transformers | Medium |
| `nn.residual` | Skip connections | Simple |
| `nn.relu` | ReLU activation | Simple |
| `nn.sigmoid` | Sigmoid activation | Simple |
| `nn.tanh` | Tanh activation | Simple |
| `nn.softmax` | Softmax activation | Simple |
| `nn.gelu` | GELU activation | Simple |

### Optimizers to Implement

| Optimizer | Algorithm |
|-----------|-----------|
| `nn.sgd` | Stochastic Gradient Descent |
| `nn.adam` | Adaptive Moment Estimation |
| `nn.adamw` | Adam with weight decay |
| `nn.rmsprop` | Root Mean Square Propagation |
| `nn.adagrad` | Adaptive Gradient |

### Loss Functions

| Loss | Use Case |
|------|----------|
| `nn.mse` | Regression |
| `nn.cross_entropy` | Multi-class classification |
| `nn.binary_cross_entropy` | Binary classification |
| `nn.huber` | Robust regression |
| `nn.cosine_loss` | Similarity learning |

### Implementation Plan

**Weeks 1-2 — Core Engine:**
1. Tensor autograd (automatic differentiation)
2. Computational graph (forward + backward pass)
3. Dense layer (forward: Wx+b, backward: gradient computation)
4. Activation functions (relu, sigmoid, tanh, softmax)
5. Loss functions (MSE, cross-entropy)
6. SGD optimizer
7. Basic training loop

**Weeks 3-4 — Essential Layers:**
8. Adam/AdamW optimizers
9. Dropout layer
10. Batch normalization
11. Flatten layer
12. Conv2d layer (im2col implementation)
13. MaxPool/AvgPool layers
14. Data batching and shuffling

**Weeks 5-6 — Recurrent & Attention:**
15. Embedding layer
16. LSTM layer
17. GRU layer
18. Multi-head attention
19. Positional encoding
20. Transformer block

**Weeks 7-8 — Polish:**
21. Model save/load (binary format)
22. Learning rate schedulers
23. Model summary
24. Training history and metrics
25. Data augmentation
26. GPU acceleration via CUDA (optional)

### C++ Backend Structure
```cpp
namespace pyro_nn {
    struct Tensor {
        std::vector<double> data;
        std::vector<double> grad;
        std::vector<int64_t> shape;
        // autograd
        std::function<void()> backward_fn;
    };

    struct Layer {
        virtual Tensor forward(const Tensor& input) = 0;
        virtual Tensor backward(const Tensor& grad_output) = 0;
        virtual std::vector<Tensor*> params() { return {}; }
    };

    struct Dense : Layer { Tensor weights, bias; /* ... */ };
    struct Conv2d : Layer { Tensor filters, bias; /* ... */ };
    struct LSTM : Layer { Tensor Wf, Wi, Wc, Wo, bf, bi, bc, bo; /* ... */ };

    struct Sequential {
        std::vector<std::unique_ptr<Layer>> layers;
        Tensor forward(const Tensor& input);
        void fit(const Tensor& x, const Tensor& y, int epochs, int batch);
    };
}
```

### Tests Needed
- Dense layer: forward + backward (2 tests)
- Activations: relu, sigmoid, tanh, softmax (4 tests)
- Conv2d: forward + backward (2 tests)
- LSTM: forward + backward (2 tests)
- Attention: forward (1 test)
- Optimizers: SGD, Adam (2 tests)
- Loss: MSE, cross_entropy (2 tests)
- Training: full training loop convergence test (1 test)
- Save/load (1 test)
- Model summary (1 test)
- **Total: 18 tests**

---

## Phase 4 — Natural Language Processing & Computer Vision

**Timeline:** 4-6 weeks (after Phase 3)
**Priority:** Medium-High
**Modules:** `nlp`, `cv`

### NLP Module API

```pyro
import nlp

# === TOKENIZATION ===
tokens = nlp.tokenize("Hello, how are you?")
# ["Hello", ",", "how", "are", "you", "?"]

words = nlp.word_tokenize("The quick brown fox")
# ["The", "quick", "brown", "fox"]

sents = nlp.sent_tokenize("Hello. How are you? I'm fine.")
# ["Hello.", "How are you?", "I'm fine."]

# === TEXT PREPROCESSING ===
clean = nlp.lowercase("Hello World")         # "hello world"
clean = nlp.remove_punctuation("Hello!")      # "Hello"
stems = nlp.stem(["running", "jumped"])       # ["run", "jump"]
stops = nlp.remove_stopwords("the cat is on the mat")  # "cat mat"

# === TEXT VECTORIZATION ===
bow = nlp.bag_of_words(documents)
tfidf = nlp.tfidf(documents)
vec = nlp.word2vec(corpus, dim=100, window=5)
embed = vec.embed("king")  # 100-dim vector
similar = vec.most_similar("king", 5)
# [("queen", 0.89), ("prince", 0.82), ...]

# Word arithmetic
result = vec.analogy("king", "man", "woman")  # "queen"

# === SENTIMENT ANALYSIS ===
result = nlp.sentiment("Pyro is amazing!")
print(result.label)  # "positive"
print(result.score)  # 0.97

result = nlp.sentiment("This is terrible")
print(result.label)  # "negative"
print(result.score)  # 0.92

# === NAMED ENTITY RECOGNITION ===
entities = nlp.ner("Aravind Pilla lives in Hyderabad and works at Synchrony")
# [{text: "Aravind Pilla", type: "PERSON"},
#  {text: "Hyderabad", type: "LOCATION"},
#  {text: "Synchrony", type: "ORGANIZATION"}]

# === TEXT SIMILARITY ===
score = nlp.similarity("Pyro is fast", "Pyro has good performance")
print(score)  # 0.89

# === TEXT CLASSIFICATION ===
classifier = nlp.text_classifier(train_texts, train_labels)
prediction = classifier.predict("new text to classify")

# === TEXT GENERATION ===
model = nlp.language_model(corpus)
generated = model.generate("Once upon a time", max_length=100)

# === SUMMARIZATION ===
summary = nlp.summarize(long_text, sentences=3)

# === KEYWORD EXTRACTION ===
keywords = nlp.keywords(text, top=10)
```

### Computer Vision Module API

```pyro
import cv

# === IMAGE LOADING ===
img = cv.load("photo.jpg")
print(img.width)       # 1920
print(img.height)      # 1080
print(img.channels)    # 3 (RGB)

# === IMAGE OPERATIONS ===
resized = cv.resize(img, 224, 224)
gray = cv.grayscale(img)
flipped = cv.flip(img, "horizontal")
rotated = cv.rotate(img, 90)
cropped = cv.crop(img, x=100, y=100, w=200, h=200)
blurred = cv.blur(img, radius=5)
sharp = cv.sharpen(img)

# === FILTERS & EDGE DETECTION ===
edges = cv.canny(gray, low=50, high=150)
sobel = cv.sobel(gray)
thresh = cv.threshold(gray, 128)

# === COLOR OPERATIONS ===
hsv = cv.to_hsv(img)
rgb = cv.to_rgb(hsv)
r, g, b = cv.split_channels(img)
merged = cv.merge_channels(r, g, b)

# === DRAWING ===
cv.draw_rect(img, x=10, y=10, w=100, h=50, color="red")
cv.draw_circle(img, cx=100, cy=100, radius=50, color="blue")
cv.draw_text(img, "Hello", x=10, y=30, size=24, color="white")
cv.draw_line(img, x1=0, y1=0, x2=100, y2=100, color="green")

# === SAVE ===
cv.save(img, "output.png")
cv.save(img, "output.jpg", quality=90)

# === OBJECT DETECTION (pre-trained) ===
objects = cv.detect(img)
for obj in objects
    print("{obj.label}: {obj.confidence} at ({obj.x}, {obj.y})")

# === IMAGE CLASSIFICATION (pre-trained) ===
result = cv.classify(img)
print(result.label)      # "golden retriever"
print(result.confidence) # 0.94

# === FACE DETECTION ===
faces = cv.detect_faces(img)
for face in faces
    print("Face at ({face.x}, {face.y}) size {face.w}x{face.h}")

# === IMAGE TO TENSOR (for neural networks) ===
tensor = cv.to_tensor(img)        # shape: [3, 224, 224]
tensor = cv.normalize(tensor, mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
```

### Implementation Plan

**NLP (Weeks 1-3):**
1. Tokenizer (regex-based word/sentence splitting)
2. Text preprocessing (lowercase, punctuation, stopwords, stemming)
3. Bag of words + TF-IDF
4. Word2Vec (Skip-gram with negative sampling, train from scratch)
5. Cosine similarity
6. Sentiment analysis (naive bayes classifier pre-trained on movie reviews)
7. Named entity recognition (rule-based + pattern matching)
8. Text classifier (TF-IDF + logistic regression)
9. Keyword extraction (TF-IDF top-k)
10. Summarization (extractive — sentence scoring)

**CV (Weeks 4-6):**
11. Image loading/saving (PPM natively, JPEG/PNG via stb_image header library)
12. Resize (bilinear interpolation)
13. Grayscale, color conversions
14. Flip, rotate, crop
15. Blur (Gaussian), sharpen
16. Edge detection (Canny, Sobel)
17. Threshold (binary, adaptive)
18. Drawing (rect, circle, text, line)
19. Image to tensor conversion
20. Pre-trained classification (bundled small model or ONNX runtime)

### Tests Needed
- NLP: tokenize, tfidf, sentiment, similarity, ner (5 tests)
- CV: load, resize, grayscale, canny, threshold (5 tests)
- **Total: 10 tests**

---

## Phase 5 — LLM Integration (Built-in AI)

**Timeline:** 1 week
**Priority:** HIGHEST — unique selling point, you already have the NVIDIA API
**Module:** `ai`

### What It Does
Built-in LLM access with support for 10+ providers. Configure once with `ai.provider(name, key)` and start building.

### API Design

```pyro
import ai

# === CHAT ===
response = ai.chat("Explain quantum computing simply")
print(response)

# With specific model
response = ai.chat("Write a haiku about coding", model="llama-3.3-70b")

# With system prompt
response = ai.chat("Translate to French: Hello world",
    system="You are a professional translator")

# === STREAMING ===
for chunk in ai.stream("Tell me a long story about a robot")
    print(chunk)

# === CONVERSATION ===
convo = ai.conversation(system="You are a helpful coding tutor")
answer1 = convo.ask("What is recursion?")
answer2 = convo.ask("Show me an example in Pyro")
answer3 = convo.ask("Now explain the time complexity")

# === EMBEDDINGS ===
vec = ai.embed("Hello world")
print(vec.shape)  # [4096]

# Compare similarity
score = ai.similarity("cat", "dog")
print(score)  # 0.82

# === RAG (Retrieval Augmented Generation) ===
kb = ai.knowledge_base()
kb.add("docs/manual.txt")
kb.add("docs/api.txt")
kb.add("docs/faq.txt")
kb.add_text("Pyro was created by Aravind Pilla")

answer = kb.ask("How do I install Pyro?")
print(answer)  # answers based on your documents

# === STRUCTURED OUTPUT ===
result = ai.extract("John is 25 and lives in NYC",
    fields=["name", "age", "city"])
print(result)  # {name: "John", age: 25, city: "NYC"}

# === CODE GENERATION ===
code = ai.generate_code("sort a list of numbers in Pyro")
print(code)

# === SUMMARIZATION ===
summary = ai.summarize(long_text, max_words=100)

# === TRANSLATION ===
french = ai.translate("Hello, how are you?", to="french")

# === CLASSIFICATION ===
label = ai.classify("I love this product!", labels=["positive", "negative", "neutral"])
print(label)  # "positive"

# === IMAGE UNDERSTANDING (multimodal) ===
description = ai.describe_image("photo.jpg")
answer = ai.ask_image("photo.jpg", "What color is the car?")
```

### Implementation Plan

**Day 1-2 — Core Chat:**
1. HTTP client to NVIDIA API (reuse existing PyroBot backend approach)
2. `ai.chat()` with model selection
3. `ai.stream()` with chunked response
4. System prompt support
5. Conversation history management

**Day 3-4 — Embeddings & RAG:**
6. `ai.embed()` using NVIDIA embedding API
7. `ai.similarity()` using cosine distance
8. `ai.knowledge_base()` with text chunking
9. Vector storage (in-memory, simple cosine search)
10. `kb.ask()` — retrieve relevant chunks, inject into prompt

**Day 5-6 — Utility Functions:**
11. `ai.summarize()` — prompt-based
12. `ai.translate()` — prompt-based
13. `ai.classify()` — prompt-based with constrained output
14. `ai.extract()` — structured extraction via prompt
15. `ai.generate_code()` — code generation prompt

**Day 7 — Polish:**
16. Error handling (network errors, API limits, timeouts)
17. Response caching (optional)
18. Model list and selection
19. Documentation and examples

### C++ Backend
```cpp
namespace pyro_ai {
    // Uses libcurl for HTTP, nlohmann/json for parsing
    struct Response {
        std::string text;
        int tokens_used;
    };

    Response chat(const std::string& prompt,
                  const std::string& model = "meta/llama-3.3-70b-instruct",
                  const std::string& system = "");

    std::vector<double> embed(const std::string& text);

    struct KnowledgeBase {
        std::vector<std::pair<std::string, std::vector<double>>> chunks;
        void add(const std::string& file_path);
        void add_text(const std::string& text);
        std::string ask(const std::string& question);
    };
}
```

### Configuration
The NVIDIA API key is bundled in the compiler (for Aravind's distribution) or can be set via environment variable:
```
export PYRO_AI_KEY="nvapi-xxxxx"
```
Or in code:
```pyro
ai.set_key("nvapi-xxxxx")
```

### Tests Needed
- Chat: basic response (1 test)
- Streaming: chunked output (1 test)
- Conversation: multi-turn (1 test)
- Embed: vector output (1 test)
- Similarity: cosine score (1 test)
- KnowledgeBase: add + ask (1 test)
- Classify: label output (1 test)
- **Total: 7 tests**

---

## Phase 6 — Data Visualization for ML

**Timeline:** 2 weeks
**Priority:** Medium — complements ML workflow
**Module:** `plot` (extends existing `viz`)

### What It Does
Publication-quality charts for ML workflows. Training curves, confusion matrices, feature distributions, scatter plots, correlation heatmaps — all generated as SVG or PNG.

### API Design

```pyro
import plot

# === BASIC CHARTS ===
plot.line(x, y, title="Training Loss", xlabel="Epoch", ylabel="Loss")
plot.scatter(x, y, title="Feature Distribution", color="blue")
plot.bar(labels, values, title="Feature Importance")
plot.histogram(data, bins=30, title="Distribution")
plot.pie(labels, values, title="Class Distribution")
plot.heatmap(matrix, title="Correlation Matrix", colormap="viridis")

# === ML-SPECIFIC CHARTS ===
plot.training_curve(history, metrics=["loss", "val_loss"])
plot.confusion_matrix(y_true, y_pred, labels=["cat", "dog", "bird"])
plot.roc_curve(y_true, y_scores)
plot.precision_recall_curve(y_true, y_scores)
plot.feature_importance(names, scores, top=15)
plot.learning_rate(scheduler_history)
plot.cluster_plot(data, labels, method="tsne")
plot.decision_boundary(model, x, y)

# === CUSTOMIZATION ===
plot.set_style("dark")
plot.set_size(800, 600)
plot.set_font("monospace")

# === SUBPLOTS ===
fig = plot.figure(rows=2, cols=2)
fig.add(0, 0, plot.line(x1, y1, title="Loss"))
fig.add(0, 1, plot.line(x2, y2, title="Accuracy"))
fig.add(1, 0, plot.scatter(x3, y3, title="Predictions"))
fig.add(1, 1, plot.bar(names, scores, title="Features"))
fig.save("dashboard.svg")

# === SAVE ===
plot.save("chart.svg")
plot.save("chart.png")
plot.show()  # opens in browser
```

### Implementation
Extends the existing `viz` module (SVG chart generation) with ML-specific chart types.

---

## Summary — Full Roadmap

| Phase | Module | Timeline | Effort | Impact |
|-------|--------|----------|--------|--------|
| **5** | `ai` | **1 week** | Low | **Massive** — first language with built-in LLM |
| **2** | `ml` | 3-4 weeks | Medium | Very High — scikit-learn replacement |
| **1** | `tensor` | 2-3 weeks | Medium | High — foundation for everything |
| **3** | `nn` | 6-8 weeks | High | Massive — TensorFlow/PyTorch alternative |
| **6** | `plot` | 2 weeks | Low | Medium — ML visualization |
| **4** | `nlp` + `cv` | 4-6 weeks | Medium | High — NLP + computer vision |

### Recommended Build Order
```
Phase 5 (ai)      ← START HERE — 1 week, massive differentiation
  ↓
Phase 1 (tensor)  ← Foundation for ML + DL
  ↓
Phase 2 (ml)      ← Classical ML (most users need this)
  ↓
Phase 6 (plot)    ← Visualization for ML results
  ↓
Phase 3 (nn)      ← Deep learning
  ↓
Phase 4 (nlp+cv)  ← NLP + Computer Vision
```

### Total New Tests: 84
### Total Timeline: 18-24 weeks (4-6 months)
### End Result: Pyro becomes the first language where you can go from data loading to trained neural network to LLM-powered app — with zero external dependencies.

---

*"The language that makes C++ simple, Python jealous, and pip install obsolete."*
