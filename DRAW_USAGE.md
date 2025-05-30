# Programa de Visualização - draw.cpp

Este programa lê uma entrada de malha e gera visualizações SVG da entrada original e, se a malha for válida, também da estrutura DCEL resultante.

## Compilação

```bash
g++ -o draw draw.cpp -std=c++17
```

## Uso

```bash
./draw < arquivo_entrada.txt
```

## Formato de Entrada

O programa espera o mesmo formato de entrada do programa `malha`:

```
n f
x1 y1
x2 y2
...
xn yn
v1 v2 ... vk
v1 v2 ... vj
...
```

Onde:

- `n`: número de vértices
- `f`: número de faces
- `xi yi`: coordenadas inteiras do vértice i
- Cada linha seguinte define uma face com seus vértices (índices 1-indexados)

## Saída

O programa gera dois tipos de arquivos SVG:

### 1. input_mesh.svg

- **Sempre gerado**: Visualização da malha de entrada
- **Elementos visuais**:
  - Polígonos coloridos representando as faces
  - Círculos vermelhos nos vértices
  - Números identificando cada vértice
  - Legenda com informações da malha

### 2. dcel_structure.svg

- **Gerado apenas se a malha for válida**: Visualização da estrutura DCEL
- **Elementos visuais**:
  - Linhas azuis com setas representando half-edges direcionadas
  - Números azuis identificando cada half-edge
  - Círculos vermelhos nos vértices com números
  - Labels verdes "F1", "F2", etc. identificando faces
  - Legenda explicativa

## Exemplos de Uso

### Exemplo 1: Malha Válida (Quadrado)

```bash
# Criar arquivo test_square.txt:
4 2
0 0
2 0
2 2
0 2
1 2 3 4
4 3 2 1

# Executar:
./draw < test_square.txt
```

**Resultado**: Gera `input_mesh.svg` e `dcel_structure.svg`

### Exemplo 2: Malha Inválida (Triângulo Aberto)

```bash
# Criar arquivo test_triangle.txt:
3 1
0 0
3 0
1 2
1 2 3

# Executar:
./draw < test_triangle.txt
```

**Resultado**: Gera apenas `input_mesh.svg` (malha é "aberta")

## Interpretação das Visualizações

### Input Mesh (input_mesh.svg)

- **Faces**: Polígonos coloridos com transparência
- **Vértices**: Círculos vermelhos numerados
- **Orientação**: Mostra a ordem dos vértices conforme especificado na entrada

### DCEL Structure (dcel_structure.svg)

- **Half-edges**: Setas azuis mostrando direção e conectividade
- **Numeração**: Cada half-edge tem um número (1-indexado)
- **Faces**: Labels verdes no centro geométrico de cada face
- **Vértices**: Círculos vermelhos com bordas mais espessas
- **Orientação**: Setas mostram a orientação counter-clockwise das faces

## Validação de Malhas

O programa utiliza o executável `malha` para validar a entrada. Os possíveis resultados são:

1. **Malha Válida**: Gera DCEL e ambas as visualizações
2. **"aberta"**: Alguma aresta borda apenas uma face
3. **"não subdivisão planar"**: Alguma aresta borda mais de duas faces
4. **"superposta"**: Faces se auto-intersectam ou se sobrepõem

## Visualização no Navegador

Os arquivos SVG podem ser abertos diretamente em qualquer navegador web moderno:

```bash
# macOS
open input_mesh.svg
open dcel_structure.svg

# Linux
xdg-open input_mesh.svg
xdg-open dcel_structure.svg

# Windows
start input_mesh.svg
start dcel_structure.svg
```

## Características Técnicas

- **Coordenadas**: Suporta coordenadas inteiras conforme especificação
- **Escala Automática**: Ajusta automaticamente o zoom para mostrar toda a malha
- **Cores**: Faces têm cores diferentes para facilitar distinção
- **Orientação**: Coordenadas Y são invertidas para corresponder ao padrão SVG
- **Robustez**: Trata casos especiais como malhas degeneradas

## Limitações

- Requer que o executável `malha` esteja no mesmo diretório
- Funciona apenas com coordenadas inteiras
- Visualização pode ficar confusa com malhas muito complexas (muitas faces/arestas)
- Half-edges muito próximas podem ter labels sobrepostas

## Troubleshooting

### Erro: "command not found"

- Verifique se compilou corretamente: `g++ -o draw draw.cpp -std=c++17`

### Erro: "malha: command not found"

- Verifique se o executável `malha` está no mesmo diretório
- Compile o malha: `g++ -o malha malha.cpp -std=c++17`

### SVG não abre

- Verifique se o navegador suporta SVG
- Tente abrir o arquivo em um editor de texto para ver se foi gerado corretamente

### Visualização muito pequena/grande

- O programa ajusta automaticamente, mas malhas com coordenadas muito extremas podem ter problemas
- Considere normalizar as coordenadas da entrada
