import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:go_router/go_router.dart';
import 'package:intl/intl.dart';
import '../../core/theme/app_theme.dart';
import '../../domain/entities/entities.dart' hide SessionState;
import '../../data/datasources/mock_datasource.dart';
import '../blocs/session_bloc.dart';
import '../widgets/components.dart';

// ─────────────────────────────────────────
// HOME SCREEN
// ─────────────────────────────────────────
class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(20),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              // Header
              Row(
                children: [
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        DateFormat('EEEE, d MMM').format(DateTime.now()),
                        style: const TextStyle(fontSize: 13, color: AppColors.textTertiary),
                      ),
                      const Text('Sprint Timer',
                          style: TextStyle(
                              fontSize: 24,
                              fontWeight: FontWeight.w700,
                              color: AppColors.textPrimary)),
                    ],
                  ),
                  const Spacer(),
                  GestureDetector(
                    onTap: () => context.push('/connection'),
                    child: Container(
                      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
                      decoration: BoxDecoration(
                        color: AppColors.green.withValues(alpha: 0.12),
                        borderRadius: BorderRadius.circular(20),
                        border: Border.all(color: AppColors.green.withValues(alpha: 0.3), width: 0.8),
                      ),
                      child: const Row(
                        children: [
                          Icon(Icons.circle, size: 7, color: AppColors.green),
                          SizedBox(width: 6),
                          Text('Mock activo',
                              style: TextStyle(fontSize: 12, color: AppColors.green, fontWeight: FontWeight.w600)),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 24),

              // Quick actions
              Row(
                children: [
                  Expanded(
                    child: _ActionButton(
                      label: 'Nueva sesión',
                      icon: Icons.play_arrow_rounded,
                      color: AppColors.cyan,
                      onTap: () => context.push('/new-session'),
                    ),
                  ),
                  const SizedBox(width: 10),
                  Expanded(
                    child: _ActionButton(
                      label: 'Atletas',
                      icon: Icons.group_rounded,
                      color: AppColors.violet,
                      onTap: () => context.push('/athletes'),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 10),
              Row(
                children: [
                  Expanded(
                    child: _ActionButton(
                      label: 'Historial',
                      icon: Icons.history_rounded,
                      color: AppColors.textSecondary,
                      onTap: () => context.push('/history'),
                    ),
                  ),
                  const SizedBox(width: 10),
                  Expanded(
                    child: _ActionButton(
                      label: 'Ajustes',
                      icon: Icons.tune_rounded,
                      color: AppColors.textSecondary,
                      onTap: () => context.push('/settings'),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 28),

              // Recent sessions
              const Text('Últimas sesiones',
                  style: TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.w600,
                      color: AppColors.textPrimary)),
              const SizedBox(height: 12),

              BlocBuilder<SessionBloc, SessionState>(
                builder: (context, state) {
                  if (state is SessionsLoading) {
                    return const Center(child: CircularProgressIndicator(color: AppColors.cyan));
                  }
                  if (state is SessionsLoaded) {
                    final recent = state.sessions.take(5).toList();
                    if (recent.isEmpty) {
                      return const _EmptyState(
                        icon: Icons.timer_outlined,
                        message: 'Aún no hay sesiones.\nInicia una nueva carrera.',
                      );
                    }
                    return Column(
                      children: recent.map((s) => SessionTile(
                        session: s,
                        athleteName: mockAthletes
                            .firstWhere((a) => a.athleteId == s.athleteId,
                                orElse: () => Athlete(athleteId: s.athleteId, name: '#${s.athleteId}'))
                            .name,
                        onTap: () => context.push('/session/${s.sessionId}'),
                      )).toList(),
                    );
                  }
                  return const SizedBox();
                },
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _ActionButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final Color color;
  final VoidCallback onTap;

  const _ActionButton({
    required this.label,
    required this.icon,
    required this.color,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.symmetric(vertical: 16, horizontal: 16),
        decoration: BoxDecoration(
          color: AppColors.surface,
          borderRadius: BorderRadius.circular(12),
          border: Border.all(color: AppColors.border, width: 0.5),
        ),
        child: Row(
          children: [
            Icon(icon, color: color, size: 22),
            const SizedBox(width: 10),
            Text(label,
                style: const TextStyle(
                    fontSize: 14,
                    fontWeight: FontWeight.w600,
                    color: AppColors.textPrimary)),
          ],
        ),
      ),
    );
  }
}

class _EmptyState extends StatelessWidget {
  final IconData icon;
  final String message;

  const _EmptyState({required this.icon, required this.message});

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 40),
        child: Column(
          children: [
            Icon(icon, size: 48, color: AppColors.textTertiary),
            const SizedBox(height: 12),
            Text(message,
                textAlign: TextAlign.center,
                style: const TextStyle(color: AppColors.textTertiary, fontSize: 14)),
          ],
        ),
      ),
    );
  }
}

// ─────────────────────────────────────────
// ATHLETES SCREEN
// ─────────────────────────────────────────
class AthletesScreen extends StatefulWidget {
  const AthletesScreen({super.key});

  @override
  State<AthletesScreen> createState() => _AthletesScreenState();
}

class _AthletesScreenState extends State<AthletesScreen> {
  String _query = '';

  @override
  Widget build(BuildContext context) {
    final filtered = mockAthletes
        .where((a) =>
            a.name.toLowerCase().contains(_query.toLowerCase()) ||
            '${a.athleteId}'.contains(_query))
        .toList();

    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('Atletas'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(20, 0, 20, 12),
            child: TextField(
              onChanged: (v) => setState(() => _query = v),
              style: const TextStyle(color: AppColors.textPrimary),
              decoration: const InputDecoration(
                hintText: 'Buscar por nombre o ID...',
                prefixIcon: Icon(Icons.search, color: AppColors.textTertiary, size: 20),
              ),
            ),
          ),
          Expanded(
            child: filtered.isEmpty
                ? _EmptyState(
                    icon: Icons.person_search,
                    message: 'No se encontraron atletas con "$_query"')
                : ListView.builder(
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                    itemCount: filtered.length,
                    itemBuilder: (_, i) {
                      final a = filtered[i];
                      return GestureDetector(
                        onTap: () => context.push('/athlete/${a.athleteId}'),
                        child: Container(
                          margin: const EdgeInsets.only(bottom: 8),
                          padding: const EdgeInsets.all(16),
                          decoration: BoxDecoration(
                            color: AppColors.surface,
                            borderRadius: BorderRadius.circular(12),
                            border: Border.all(color: AppColors.border, width: 0.5),
                          ),
                          child: Row(
                            children: [
                              Container(
                                width: 44,
                                height: 44,
                                decoration: BoxDecoration(
                                  color: AppColors.violet.withValues(alpha: 0.15),
                                  shape: BoxShape.circle,
                                ),
                                child: Center(
                                  child: Text('#${a.athleteId}',
                                      style: const TextStyle(
                                          fontSize: 14,
                                          fontWeight: FontWeight.w700,
                                          color: AppColors.violet)),
                                ),
                              ),
                              const SizedBox(width: 14),
                              Expanded(
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    Text(a.name,
                                        style: const TextStyle(
                                            fontSize: 15,
                                            fontWeight: FontWeight.w600,
                                            color: AppColors.textPrimary)),
                                    if (a.notes != null)
                                      Text(a.notes!,
                                          style: const TextStyle(
                                              fontSize: 12,
                                              color: AppColors.textTertiary)),
                                  ],
                                ),
                              ),
                              const Icon(Icons.chevron_right,
                                  color: AppColors.textTertiary, size: 20),
                            ],
                          ),
                        ),
                      );
                    },
                  ),
          ),
        ],
      ),
    );
  }
}

// ─────────────────────────────────────────
// HISTORY SCREEN
// ─────────────────────────────────────────
class HistoryScreen extends StatefulWidget {
  const HistoryScreen({super.key});

  @override
  State<HistoryScreen> createState() => _HistoryScreenState();
}

class _HistoryScreenState extends State<HistoryScreen> {
  int? _athleteId;
  int? _distance;

  final _athletes = mockAthletes.map((a) => (id: a.athleteId, name: a.name)).toList();

  void _applyFilters() {
    context.read<SessionBloc>().add(LoadSessions(
      athleteId: _athleteId,
      distanceMeters: _distance,
    ));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('Historial'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(20, 0, 20, 12),
            child: FilterBar(
              selectedAthleteId: _athleteId,
              selectedDistance: _distance,
              athletes: _athletes,
              onAthleteChanged: (v) { setState(() => _athleteId = v); _applyFilters(); },
              onDistanceChanged: (v) { setState(() => _distance = v); _applyFilters(); },
              onClearFilters: () {
                setState(() { _athleteId = null; _distance = null; });
                _applyFilters();
              },
            ),
          ),
          Expanded(
            child: BlocBuilder<SessionBloc, SessionState>(
              builder: (context, state) {
                if (state is SessionsLoading) {
                  return const Center(child: CircularProgressIndicator(color: AppColors.cyan));
                }
                if (state is SessionsLoaded) {
                  if (state.sessions.isEmpty) {
                    return const _EmptyState(
                      icon: Icons.history,
                      message: 'No hay sesiones con los filtros actuales.',
                    );
                  }
                  return ListView.builder(
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                    itemCount: state.sessions.length,
                    itemBuilder: (_, i) {
                      final s = state.sessions[i];
                      return SessionTile(
                        session: s,
                        athleteName: mockAthletes
                            .firstWhere((a) => a.athleteId == s.athleteId,
                                orElse: () => Athlete(athleteId: s.athleteId, name: '#${s.athleteId}'))
                            .name,
                        onTap: () => context.push('/session/${s.sessionId}'),
                      );
                    },
                  );
                }
                if (state is SessionsError) {
                  return Center(child: Text(state.message,
                      style: const TextStyle(color: AppColors.red)));
                }
                return const SizedBox();
              },
            ),
          ),
        ],
      ),
    );
  }
}
