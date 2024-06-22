#include <cstdint>
#include <cstdio>
#include <utility>
#include <vector>

using std::pair;
using std::vector;

/// @brief Transforma o bit `bit` do número `number` em 1
/// @param number O número a ser modificado
/// @param bit O índice (0-based) do bit a ser ativado
inline void SET_BIT(int &number, int bit) { number |= (1 << bit); }

/// @brief Transforma o bit `bit` do número `number` em 0
/// @param number O número a ser modificado
/// @param bit O índice (0-based) do bit a ser desativado
inline void CLEAR_BIT(int &number, int bit) { number &= (~(1 << bit)); }

/// @brief Retorna o valor do bit `bit` do número `number`
/// @param number O número de onde o bit será extraído
/// @param bit O índice (0-based) do bit a ser retornado
/// @return O valor do bit passado (0 ou 1)
inline int GET_BIT(int number, int bit) { return (number >> bit) & 0b1; }

/// @brief Reresenta um cristal da caixa.
struct Cristal {
  // O brilho do cristal. Um valor de -1 indica que não existe um cristal
  // naquela posição.
  int brilho = -1;

  // Armazena as conexões do cristal com os cristais ao seu redor. Os bits 0, 1,
  // 2 e 3 estarão ativados se o cristal está conectado com o cristal à sua
  // direita, acima, à sua esquerda e abaixo, respectivamente.
  int conexoes = 0;
};

/// @brief Representa uma resposta da programação dinâmica para um estado
/// específico.
struct Resposta {
  // Indica se a resposta para o estado correspondente já foi calculada ou não.
  bool calculado = false;

  // Valor da maior soma de cristais que pôde ser encontrada utilizando o estado
  // atual. Um valor de -1 indica que
  // - a configuração da linha é inválida, ou
  // - a utilizção desta configuração irá levar invariavelmente a um estado
  // inválido.
  int valor = -1;

  // Indica qual configuração da linha anterior levou à maior soma encontrada
  // (armazenada em `valor`)
  int conf = 0;
};

/// @brief Representa e resolve um problema da Cifra Carmesim.
class Cifra {
 public:
  /// @brief Constroi um problema da Cifra Carmesim.
  /// @param l O número de linhas da caixa
  /// @param c O número de colunas da caixa
  /// @param n O número de cristais da caixa
  Cifra(int l, int c, int n)
      : L_(l), C_(c), N_(n), num_possibilidades_(0b1 << c) {
    // Inicializa a caixa e a memoização da programação dinâmica para matrizes
    // vazias
    caixa_.assign(L_, vector<Cristal>(C_, Cristal()));
    memo_.assign(L_, vector<vector<Resposta>>(
                         num_possibilidades_,
                         vector<Resposta>(num_possibilidades_, Resposta())));
  }

  /// @brief Adiciona um cristal à caixa na posição (`x`, `y`)
  /// @param x A linha onde fica o cristal (1-based)
  /// @param y A coluna onde fica o cristal (1-based)
  /// @param v O valor de brilho do cristal
  /// @param d Indica se o cristal está conectado com o cristal à sua direita
  /// @param c Indica se o cristal está conectado com o cristal acima
  /// @param e Indica se o cristal está conectado com o cristal à sua esquerda
  /// @param b Indica se o cristal está conectado com o cristal abaixo
  /// @attention O par (x, y) significa linha x e coluna y, não são coordenadas
  /// cartesianas. As coordenadas x e y são iniciadas em 1, não em 0. Os
  /// indicadores de conexões (d, c, e, b) devem receber apenas valores de 1 (se
  /// a conexão existe) ou 0 (caso contrário).
  void AdicionaCristal(int x, int y, int v, int d, int c, int e, int b) {
    int conexoes = 0;
    d == 1 ? SET_BIT(conexoes, 0) : CLEAR_BIT(conexoes, 0);
    c == 1 ? SET_BIT(conexoes, 1) : CLEAR_BIT(conexoes, 1);
    e == 1 ? SET_BIT(conexoes, 2) : CLEAR_BIT(conexoes, 2);
    b == 1 ? SET_BIT(conexoes, 3) : CLEAR_BIT(conexoes, 3);

    // As coordenadas passadas são baseadas em 1, então um ajuste é feito ára
    // caber nas dimensões da matriz
    caixa_[x - 1][y - 1] = {v, conexoes};
  }

  /// @brief Resolve o problema da caixa representada. Deve ser chamado apenas
  /// quando todos os cristais já tiverem sido adicionados, e antes que qualquer
  /// informação sobre a solução seja consultada.
  void Resolve() {
    // Encontra a combinação da linha inicial que retorna a maior soma
    int conf_inicial_maxima = -1;
    Resposta maximo = {true, -1, 0};
    for (int i = 0; i < num_possibilidades_; i++) {
      Resposta resp = f(L_ - 1, i, i);

      if (resp.valor > maximo.valor) {
        maximo = resp;
        conf_inicial_maxima = i;
      }
    }

    max_valor_caixa_ = maximo.valor;

    // Percorre a tabela encontrando a combinação ótima para cada linha
    int conf = conf_inicial_maxima;
    for (int i = L_ - 1; i >= 0; i--) {
      for (int j = C_ - 1; j >= 0; j--) {
        if (GET_BIT(conf, j) == 1) {
          num_cristais_usados_++;
          cristais_solucao_.push_back({i + 1, j + 1});
        }
      }

      conf = memo_[(i + L_) % L_][conf][conf_inicial_maxima].conf;
    }
  }

  /// @brief Retorna o número de cristais usados na solução e a soma de seus
  /// brilhos.
  /// @return Um par de `int`s onde o primeiro é o número de cristais usados e o
  /// segundo é soma dos seus brilhos
  pair<int, int> GetValoresSolucao() {
    return {num_cristais_usados_, max_valor_caixa_};
  }

  /// @brief Retorna uma lista dos cristais usados na solução
  /// @return Um vector de pares (x, y) representando a posição de cada cristal
  /// utilizado
  vector<pair<int, int>> &GetCristaisSolucao() { return cristais_solucao_; }

 private:
  int L_ = 0, C_ = 0, N_ = 0;

  /// @brief Número de configurações possíveis que uma linha da caixa pode
  /// assumir (2**C_).
  int num_possibilidades_ = 0;

  /// @brief Número de cristais utilizados na solução
  int num_cristais_usados_ = 0;

  /// @brief Valor da solução ótima da caixa.
  int max_valor_caixa_ = 0;

  /// @brief Lista de cristais utilizados na solução
  vector<pair<int, int>> cristais_solucao_;

  /// @brief Matriz `L_`x`C_` dos cristais do problema
  vector<vector<Cristal>> caixa_;

  /// @brief Matriz `L_`x`num_possibiliddes_`x`num_possibiliddes_` de memoização
  /// da função de programação dinâmica
  vector<vector<vector<Resposta>>> memo_;

  /// @brief Programação dinâmica que encontra a maior soma de cristais da
  /// caixa, dado uma configuração inicial e uma configuração de linha.
  /// @param linha O índice da linha atual da caixa
  /// @param conf A configuração da linha atual da caixa
  /// @param conf_inicial A configuração utilizada na última linha da caixa
  /// neste ramo da árvore de recursão
  /// @return Uma `Resposta` onde `valor` é o maior valor encontrado e `conf` é
  /// a configuração que foi utilizada na linha acima para encontrar este
  /// máximo.
  Resposta f(int linha, int conf, int conf_inicial) {
    // Verifica memoiização
    if (memo_[linha][conf][conf_inicial].calculado) {
      return memo_[linha][conf][conf_inicial];
    }

    // Checa se a configuração é consistente
    if (!EhInternamenteConsistente(linha, conf)) {
      memo_[linha][conf][conf_inicial] = {true, -1, 0};
      return memo_[linha][conf][conf_inicial];
    }

    // Soma o valor dos cristais da linha atual
    int valor_linha = 0;
    for (int j = 0; j < C_; j++) {
      if (GET_BIT(conf, j) == 1) {
        valor_linha += caixa_[linha][j].brilho;
      }
    }

    // Caso base
    if (linha == 0) {
      // Verifica se a configuração atual e a configuração da última linha da
      // caixa são compatíveis
      if (!SaoCompativeis(linha, conf, conf_inicial)) {
        memo_[linha][conf][conf_inicial] = {true, -1, 0};
        return memo_[linha][conf][conf_inicial];
      }

      // Dado que as linhas são compatíveis, retorna o valor da linha atual
      memo_[linha][conf][conf_inicial] = {true, valor_linha, conf_inicial};
      return memo_[linha][conf][conf_inicial];
    }

    // Inicia o máximo como uma resposta inválida, pois se nenhuma possibilidade
    // para a linha acima retornou uma resposta válida, a configuração conf
    // para a linha atual também é inválida
    Resposta maximo = {true, -1, 0};

    // Testa com todas as combinações da linha acima
    for (int poss = 0; poss < num_possibilidades_; poss++) {
      // Verifica se a linha atual e a linha acima são compatíveis
      if (!SaoCompativeis(linha, conf, poss)) {
        continue;
      }

      // Faz a chamada recursiva da PD
      Resposta resp = f(linha - 1, poss, conf_inicial);

      // Se utilizar a possibilidade atual gerou um resultado inválido, pule
      if (resp.valor == -1) {
        continue;
      }

      // Encontrou um novo resultado melhor utilizando a possibilidade atual
      if (resp.valor + valor_linha > maximo.valor) {
        maximo = {true, resp.valor + valor_linha, poss};
      }
    }

    // Memoiza e retorna
    memo_[linha][conf][conf_inicial] = maximo;
    return maximo;
  }

  /// @brief Verifica se a configuração `conf` não quebra nenhuma restrição para
  /// a linha `linha`
  /// @param linha O índice linha da caixa a ser verificada
  /// @param conf A configuração de cristais ativados a ser testada para a linha
  /// dada
  /// @return `true` se a configuração é valida, `false` caso contrário.
  inline bool EhInternamenteConsistente(int linha, int conf) {
    for (int j = 0; j < C_; j++) {
      // A posição atual está ativada mas não possui um cristal
      if (GET_BIT(conf, j) == 1 && caixa_[linha][j].brilho == -1) {
        return false;
      }

      if (GET_BIT(conf, j) == 1 &&             // Posição atual está ativada
          GET_BIT(conf, (j + 1) % C_) == 1 &&  // Posição à direita está ativada
          GET_BIT(caixa_[linha][j].conexoes, 0) == 1  // Posições são conectadas
      ) {
        return false;
      }
    }

    return true;
  }

  /// @brief Verifica se a configuração `conf_i` para a linha `linha` da caixa
  /// não quebra nenhuma restrição se utilizada com a configuração `conf_s` para
  /// a linha acima.
  /// @param linha O índice da linha superior do par de linhas adjacentes a ser
  /// verificado
  /// @param conf_i A configuração da linha inferior do par a ser verificado
  /// @param conf_s A configuração da linha superior do par a ser verificado
  /// @return `true` se as configurações são compatíveis, `false` caso
  /// contrário.
  inline bool SaoCompativeis(int linha, int conf_i, int conf_s) {
    for (int j = 0; j < C_; j++) {
      if (GET_BIT(conf_i, j) == 1 &&  // Cristal da linha inferior está ativado
          GET_BIT(conf_s, j) == 1 &&  // Cristal da linha superior está ativado
          GET_BIT(caixa_[linha][j].conexoes, 1)  // Cristais estão conectados
      ) {
        return false;
      }
    }

    return true;
  }

  /// @brief Função de depuração que imprime o conteúdo da caixa
  void DumpCaixa() {
    for (int i = 0; i < L_; i++) {
      for (int j = 0; j < C_; j++) {
        printf("%3d ", caixa_[i][j].brilho);
      }
      printf("\n");
    }
  }

  /// @brief Função de depuração que imprime o conteúdo da matriz de memoização.
  void DumpMemo() {
    for (int k = 0; k < num_possibilidades_; k++) {
      printf("Configuração Inicial: %d\n", k);

      for (int i = 0; i < L_; i++) {
        printf("\t");
        for (int j = 0; j < num_possibilidades_; j++) {
          printf("(%3d %3d %3d) ", memo_[i][j][k].calculado,
                 memo_[i][j][k].valor, memo_[i][j][k].conf);
        }
        printf("\n");
      }
      printf("\n");
    }
  }
};

int main() {
  // Leitura dos dados do problema
  int L, C, N;
  scanf("%d %d %d", &L, &C, &N);
  Cifra cifra(L, C, N);

  int x, y, v, d, c, e, b;
  for (int i = 0; i < N; i++) {
    scanf("%d %d %d %d %d %d %d", &x, &y, &v, &d, &c, &e, &b);
    cifra.AdicionaCristal(x, y, v, d, c, e, b);
  }

  // Resolve o problema utilizando programação dinâmica
  cifra.Resolve();

  // Imprime a solução do problema
  pair<int, int> valores_solucao = cifra.GetValoresSolucao();
  printf("%d %d\n", valores_solucao.first, valores_solucao.second);

  vector<pair<int, int>> &cristais_solucao = cifra.GetCristaisSolucao();
  for (pair<int, int> cristal : cristais_solucao) {
    printf("%d %d\n", cristal.first, cristal.second);
  }

  return 0;
}
