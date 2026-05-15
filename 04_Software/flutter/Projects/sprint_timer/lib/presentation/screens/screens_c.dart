import 'package:flutter/material.dart';
import '../../core/theme/app_theme.dart';

class AboutTreximoPage extends StatelessWidget {
  const AboutTreximoPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('¿Quiénes somos?'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Tréximo',
              style: TextStyle(fontSize: 28, fontWeight: FontWeight.bold, color: AppColors.cyan),
            ),
            const SizedBox(height: 16),
            const Text(
              'Tréximo es un sistema avanzado de cronometraje deportivo inteligente diseñado para atletas de alto rendimiento. '
              'Nuestra misión es democratizar el acceso a métricas precisas que antes solo estaban disponibles para la élite.',
              style: TextStyle(fontSize: 16, color: AppColors.textSecondary, height: 1.5),
            ),
            const SizedBox(height: 24),
            _buildFeature(Icons.speed, 'Precisión milimétrica', 'Medimos cada fracción de segundo de tu carrera.'),
            _buildFeature(Icons.monitor_heart, 'Integración Biomecánica', 'Registramos tu frecuencia cardíaca para analizar el esfuerzo y la recuperación.'),
            _buildFeature(Icons.analytics, 'Análisis de Eficiencia', 'Calculamos tu Eficiencia Cardiovascular (ECV) en tiempo real.'),
          ],
        ),
      ),
    );
  }

  Widget _buildFeature(IconData icon, String title, String desc) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 20),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Icon(icon, color: AppColors.violet, size: 28),
          const SizedBox(width: 16),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(title, style: const TextStyle(fontSize: 18, fontWeight: FontWeight.w600, color: AppColors.textPrimary)),
                const SizedBox(height: 4),
                Text(desc, style: const TextStyle(fontSize: 14, color: AppColors.textTertiary)),
              ],
            ),
          )
        ],
      ),
    );
  }
}

class HowItWorksPage extends StatelessWidget {
  const HowItWorksPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('¿Cómo funciona?'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: ListView(
        padding: const EdgeInsets.all(20),
        children: const [
          Text('El Ecosistema Tréximo', style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold, color: AppColors.cyan)),
          SizedBox(height: 20),
          _StepCard(
            number: '1',
            title: 'Cerebro Central',
            desc: 'El ESP32 principal procesa toda la información de los periféricos y se comunica con esta app vía WiFi.',
          ),
          _StepCard(
            number: '2',
            title: 'Sensores de Pista',
            desc: 'Los peones de salida y llegada utilizan sensores ultrasónicos para detectar el cruce exacto del atleta, apoyados por UWB para la medición precisa de distancia.',
          ),
          _StepCard(
            number: '3',
            title: 'Banda Cardíaca',
            desc: 'Una banda BLE transmite tu frecuencia cardíaca en tiempo real al cerebro para que conozcas tu BPM final y de recuperación.',
          ),
        ],
      ),
    );
  }
}

class UserManualPage extends StatelessWidget {
  const UserManualPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('Manual de Usuario'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: ListView(
        padding: const EdgeInsets.all(20),
        children: const [
          Text('Guía Rápida', style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold, color: AppColors.textPrimary)),
          SizedBox(height: 20),
          _ManualSection(
            title: 'Añadir un Atleta',
            content: 'Ve a la sección "Atletas" en el panel principal y presiona el botón circular (+) para registrar un nuevo atleta en el sistema.',
          ),
          _ManualSection(
            title: 'Iniciar Sesión',
            content: 'Enciende el hardware Tréximo. Asegúrate de estar en la misma red WiFi o conectado al punto de acceso del dispositivo. La app recibirá los datos automáticamente al finalizar la carrera.',
          ),
          _ManualSection(
            title: 'Analizar Resultados',
            content: 'Revisa el "Historial" para ver una gráfica de barras con el desempeño de los atletas y entra al detalle de cada sesión para ver la curva de recuperación cardíaca.',
          ),
        ],
      ),
    );
  }
}

class _StepCard extends StatelessWidget {
  final String number;
  final String title;
  final String desc;
  const _StepCard({required this.number, required this.title, required this.desc});

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.only(bottom: 16),
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: AppColors.border),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          CircleAvatar(
            backgroundColor: AppColors.cyan.withValues(alpha: 0.2),
            foregroundColor: AppColors.cyan,
            child: Text(number, style: const TextStyle(fontWeight: FontWeight.bold)),
          ),
          const SizedBox(width: 16),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(title, style: const TextStyle(fontSize: 18, fontWeight: FontWeight.w600, color: AppColors.textPrimary)),
                const SizedBox(height: 8),
                Text(desc, style: const TextStyle(fontSize: 14, color: AppColors.textSecondary, height: 1.4)),
              ],
            ),
          )
        ],
      ),
    );
  }
}

class _ManualSection extends StatelessWidget {
  final String title;
  final String content;
  const _ManualSection({required this.title, required this.content});

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 24),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(title, style: const TextStyle(fontSize: 18, fontWeight: FontWeight.w600, color: AppColors.violet)),
          const SizedBox(height: 8),
          Text(content, style: const TextStyle(fontSize: 15, color: AppColors.textSecondary, height: 1.5)),
        ],
      ),
    );
  }
}
